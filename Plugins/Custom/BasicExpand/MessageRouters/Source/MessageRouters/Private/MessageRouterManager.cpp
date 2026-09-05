// Copyright © 2026 张鸿源. All Rights Reserved.


#include "MessageRouterManager.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "HAL/IConsoleManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MessageRouterManager)

DECLARE_LOG_CATEGORY_EXTERN(LogMessageRouterManager, Log, All) //定义日志分类
DEFINE_LOG_CATEGORY(LogMessageRouterManager);

namespace UE
{
	namespace UMessageRouterManager
	{
		static int32 ShouldLogMessages = 0;
		static FAutoConsoleVariableRef CVarShouldLogMessages(TEXT("MessageRouterManager.LogMessages"),
		                                                     ShouldLogMessages,
		                                                     TEXT("Should messages broadcast through the MessageRouterManager be logged?"));
	}
}

// FMessageRouterListenerHandle
//
void FMessageRouterListenerHandle::Unregister()
{
	if (UMessageRouterManager* StrongSubsystem = Subsystem.Get())
	{
		StrongSubsystem->UnregisterListener(*this);
		Subsystem.Reset();
		Channel = FGameplayTag();
		ID = 0;
	}
}

// UMessageRouterManager
//
UMessageRouterManager& UMessageRouterManager::Get(const UObject* WorldContextObject)
{
	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::Assert);
	check(World);
	UMessageRouterManager* Router = UGameInstance::GetSubsystem<UMessageRouterManager>(World->GetGameInstance());
	check(Router);
	return *Router;
}

bool UMessageRouterManager::HasInstance(const UObject* WorldContextObject)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::Assert);
	UMessageRouterManager* Router = World != nullptr ? UGameInstance::GetSubsystem<UMessageRouterManager>(World->GetGameInstance()) : nullptr;
	return Router != nullptr;
}

void UMessageRouterManager::Deinitialize()
{
	ListenerMap.Reset();
	Super::Deinitialize();
}

void UMessageRouterManager::UnregisterListener(const FMessageRouterListenerHandle& Handle)
{
	if (Handle.IsValid())
	{
		check(Handle.Subsystem == this);

		UnregisterListenerInternal(Handle.Channel, Handle.ID);
	}
	else
	{
		UE_LOG(LogMessageRouterManager, Warning, TEXT("Trying to unregister an invalid Handle."));
	}
}

void UMessageRouterManager::K2_BroadcastMessage(FGameplayTag Channel, const int32& Message)
{
	checkNoEntry();
}

DEFINE_FUNCTION(UMessageRouterManager::execK2_BroadcastMessage)
{
	P_GET_STRUCT(FGameplayTag, Channel);

	Stack.MostRecentPropertyAddress = nullptr;
	Stack.StepCompiledIn<FStructProperty>(nullptr);
	void* MessagePtr = Stack.MostRecentPropertyAddress;
	FStructProperty* StructProp = CastField<FStructProperty>(Stack.MostRecentProperty);

	P_FINISH;

	if (ensure((StructProp != nullptr) && (StructProp->Struct != nullptr) && (MessagePtr != nullptr)))
	{
		P_THIS->BroadcastMessageInternal(Channel, StructProp->Struct, MessagePtr);
	}
}

void UMessageRouterManager::BroadcastMessageInternal(FGameplayTag Channel, const UScriptStruct* StructType, const void* MessageBytes)
{
	// 如果启用了，请记录该消息
	if (UE::UMessageRouterManager::ShouldLogMessages != 0)
	{
		FString* pContextString = nullptr;
#if WITH_EDITOR
		if (GIsEditor)
		{
			extern ENGINE_API FString GPlayInEditorContextString;
			pContextString = &GPlayInEditorContextString;
		}
#endif

		FString HumanReadableMessage;
		StructType->ExportText(HumanReadableMessage, MessageBytes, nullptr, nullptr, PPF_None, nullptr);
		UE_LOG(LogMessageRouterManager, Log, TEXT("BroadcastMessage(%s, %s, %s)"), pContextString ? **pContextString : *GetPathNameSafe(this), *Channel.ToString(), *HumanReadableMessage);
	}

	// 广播消息
	bool bOnInitialTag = true;
	for (FGameplayTag Tag = Channel; Tag.IsValid(); Tag = Tag.RequestDirectParent())
	{
		if (const FChannelListenerList* pList = ListenerMap.Find(Tag))
		{
			// 复制以防在处理回调时出现删除
			TArray<FMessageRouterListenerData> ListenerArray(pList->Listeners);

			for (const FMessageRouterListenerData& Listener : ListenerArray)
			{
				if (bOnInitialTag || (Listener.MatchType == EMessageRouterMatchRule::PartialMatch))
				{
					if (Listener.bHadValidType && !Listener.ListenerStructType.IsValid())
					{
						UE_LOG(LogMessageRouterManager, Warning, TEXT("Listener struct type has gone invalid on Channel %s. Removing listener from list"), *Channel.ToString());
						UnregisterListenerInternal(Channel, Listener.HandleID);
						continue;
					}

					// 接收类型必须是发送端类型的父类型，或者完全模糊的（用于内部用途）
					if (!Listener.bHadValidType || StructType->IsChildOf(Listener.ListenerStructType.Get()))
					{
						Listener.ReceivedCallback(Channel, StructType, MessageBytes);
					}
					else
					{
						UE_LOG(LogMessageRouterManager, Error, TEXT("Struct type mismatch on channel %s (broadcast type %s, listener at %s was expecting type %s)"),
						       *Channel.ToString(),
						       *StructType->GetPathName(),
						       *Tag.ToString(),
						       *Listener.ListenerStructType->GetPathName());
					}
				}
			}
		}
		bOnInitialTag = false;
	}
}

FMessageRouterListenerHandle UMessageRouterManager::RegisterListenerInternal(FGameplayTag Channel,
                                                                             TFunction<void(FGameplayTag, const UScriptStruct*, const void*)>&& Callback,
                                                                             const UScriptStruct* StructType,
                                                                             EMessageRouterMatchRule MatchType)
{
	FChannelListenerList& List = ListenerMap.FindOrAdd(Channel);

	FMessageRouterListenerData& Entry = List.Listeners.AddDefaulted_GetRef();
	Entry.ReceivedCallback = MoveTemp(Callback);
	Entry.ListenerStructType = StructType;
	Entry.bHadValidType = (StructType != nullptr);
	Entry.HandleID = ++List.HandleID;
	Entry.MatchType = MatchType;

	return FMessageRouterListenerHandle(this, Channel, Entry.HandleID);
}

void UMessageRouterManager::UnregisterListenerInternal(FGameplayTag Channel, int32 HandleID)
{
	if (FChannelListenerList* ListenerItem = ListenerMap.Find(Channel))
	{
		const int32 MatchIndex = ListenerItem->Listeners.IndexOfByPredicate(
			[ID = HandleID](const FMessageRouterListenerData& Other)
			{
				return Other.HandleID == ID;
			});

		if (MatchIndex != INDEX_NONE)
		{
			ListenerItem->Listeners.RemoveAtSwap(MatchIndex);
		}

		if (ListenerItem->Listeners.Num() == 0)
		{
			ListenerMap.Remove(Channel);
		}
	}
}
