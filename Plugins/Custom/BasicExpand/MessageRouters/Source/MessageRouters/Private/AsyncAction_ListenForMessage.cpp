// Copyright © 2026 张鸿源. All Rights Reserved.


#include "AsyncAction_ListenForMessage.h"
#include "Engine/Engine.h"

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
	if (!MessageStructType.Get() || (MessageStructType.Get() == StructType))
	{
		ReceivedMessagePayloadPtr = Payload;

		OnMessageReceived.Broadcast(this, Channel);

		ReceivedMessagePayloadPtr = nullptr;
	}

	if (!OnMessageReceived.IsBound())
	{
		//如果创建异步节点的BP对象被销毁，OnMessageReceived在调用广播后将被解绑。在这种情况下，我们可以安全地标记该接收机为待销毁状态。需要支持更主动的清理机制 FORT-340994
		SetReadyToDestroy();
	}
}
