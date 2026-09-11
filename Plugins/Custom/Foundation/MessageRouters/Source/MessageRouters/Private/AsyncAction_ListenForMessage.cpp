// Copyright © 2026 张鸿源. All Rights Reserved.


#include "AsyncAction_ListenForMessage.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Templates/UnrealTemplate.h"
#include "UObject/Stack.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AsyncAction_ListenForMessage)

UAsyncAction_ListenForMessage* UAsyncAction_ListenForMessage::ListenForMessage(UObject* WorldContextObject, FGameplayTag Channel, UScriptStruct* PayloadType, EMessageRouterMatchRule MatchType)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World) return nullptr;

	UAsyncAction_ListenForMessage* Action = NewObject<UAsyncAction_ListenForMessage>();
	Action->WorldPtr = World;
	Action->ChannelToRegister = Channel;
	Action->MessageStructType = PayloadType;
	Action->MessageMatchType = MatchType;
	Action->RegisterWithGameInstance(World);

	return Action;
}

bool UAsyncAction_ListenForMessage::GetPayload(int32& OutPayload)
{
	checkNoEntry();
	return false;
}

DEFINE_FUNCTION(UAsyncAction_ListenForMessage::execGetPayload)
{
	Stack.MostRecentPropertyAddress = nullptr;
	Stack.StepCompiledIn<FStructProperty>(nullptr);
	void* MessagePtr = Stack.MostRecentPropertyAddress;
	FStructProperty* StructProp = CastField<FStructProperty>(Stack.MostRecentProperty);
	P_FINISH;

	bool bSuccess = false;

	// 确保我们试图通过蓝图节点传递的类型与收到的消息负载类型一致。
	if ((StructProp != nullptr) && (StructProp->Struct != nullptr) && (MessagePtr != nullptr) && (StructProp->Struct == P_THIS->MessageStructType.Get()) && (P_THIS->ReceivedMessagePayloadPtr != nullptr))
	{
		StructProp->Struct->CopyScriptStruct(MessagePtr, P_THIS->ReceivedMessagePayloadPtr);
		bSuccess = true;
	}

	*(bool*)RESULT_PARAM = bSuccess;
}

void UAsyncAction_ListenForMessage::Activate()
{
	if (UWorld* World = WorldPtr.Get())
	{
		if (UMessageRouterManager::HasInstance(World))
		{
			UMessageRouterManager& Router = UMessageRouterManager::Get(World);

			TWeakObjectPtr<UAsyncAction_ListenForMessage> WeakThis(this);
			ListenerHandle = Router.RegisterListenerInternal(ChannelToRegister, [WeakThis](FGameplayTag Channel, const UScriptStruct* StructType, const void* Payload)
			                                                 {
				                                                 if (UAsyncAction_ListenForMessage* StrongThis = WeakThis.Get())
				                                                 {
					                                                 StrongThis->HandleMessageReceived(Channel, StructType, Payload);
				                                                 }
			                                                 },
			                                                 MessageStructType.Get(),
			                                                 MessageMatchType);

			return;
		}
	}

	SetReadyToDestroy();
}

void UAsyncAction_ListenForMessage::SetReadyToDestroy()
{
	ListenerHandle.Unregister();
	Super::SetReadyToDestroy();
}

void UAsyncAction_ListenForMessage::HandleMessageReceived(FGameplayTag Channel, const UScriptStruct* StructType, const void* Payload)
{
	// 分发使用监听列表副本；即使已经注销，当前轮也可能再次调用此对象。
	if (!ShouldBroadcastDelegates())
	{
		return;
	}

	if (!MessageStructType.Get() || (MessageStructType.Get() == StructType))
	{
		// 嵌套消息结束后恢复外层负载，而不是提前清空外层回调仍在使用的指针。
		TGuardValue<const void*> PayloadGuard(ReceivedMessagePayloadPtr, Payload);
		OnMessageReceived.Broadcast(this, Channel);
	}

	if (!OnMessageReceived.IsBound())
	{
		// 创建节点的蓝图对象销毁后若已无有效接收者，结束任务并注销监听。
		SetReadyToDestroy();
	}
}
