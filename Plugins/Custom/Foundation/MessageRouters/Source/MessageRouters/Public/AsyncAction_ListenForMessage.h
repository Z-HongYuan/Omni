// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "MessageRouterManager.h"
#include "MessageRouterTypes.h"
#include "Engine/CancellableAsyncAction.h"
#include "AsyncAction_ListenForMessage.generated.h"

#define UE_API MESSAGEROUTERS_API


/**
 * 代理对象引脚将在 UAsyncAction_ListenForMessage 中隐藏。用于获取触发委托的对象的引用，以便后续调用 'GetPayload'。
 * @param ActualChannel		我们接收 Payload 的实际消息通道（将始终以 Channel 开头，但如果启用了部分匹配，可能会更具体）
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAsyncGameplayMessageDelegate, UAsyncAction_ListenForMessage*, ProxyObject, FGameplayTag, ActualChannel);

/**
 * 持续监听消息的异步任务；收到消息时同步转发事件，调用 Cancel 结束监听。
 */
UCLASS(MinimalAPI, BlueprintType, meta=(HasDedicatedAsyncNode))
class UAsyncAction_ListenForMessage : public UCancellableAsyncAction
{
	GENERATED_BODY()

public:
	/**
	 * 等待在指定通道上广播的消息。
	 *
	 * @param Channel			要监听的消息通道
	 * @param PayloadType		要使用的消息结构类型（这必须与发送者广播的类型相同）
	 * @param MatchType			用于将通道与广播消息匹配的规则
	 */
	UFUNCTION(BlueprintCallable, Category = Messaging, meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true"))
	static UE_API UAsyncAction_ListenForMessage* ListenForMessage(UObject* WorldContextObject, FGameplayTag Channel, UScriptStruct* PayloadType, EMessageRouterMatchRule MatchType = EMessageRouterMatchRule::ExactMatch);

	/**
	 * 尝试将当前消息负载复制到指定的通配符中，只能在消息回调执行期间读取。
	 * 通配符的类型必须与接收消息的类型匹配。
	 *
	 * @param OutPayload	接收消息负载的通配符引用
	 * @return				如果复制成功则返回 true
	 */
	UFUNCTION(BlueprintCallable, CustomThunk, Category = Messaging, meta = (CustomStructureParam = "OutPayload"))
	UE_API bool GetPayload(UPARAM(ref) int32& OutPayload);

	DECLARE_FUNCTION(execGetPayload);

	UE_API virtual void Activate() override;
	UE_API virtual void SetReadyToDestroy() override;

public:
	/** 当在指定通道上广播消息时调用。使用 GetPayload() 读取当前消息负载。 */
	UPROPERTY(BlueprintAssignable)
	FAsyncGameplayMessageDelegate OnMessageReceived;

private:
	void HandleMessageReceived(FGameplayTag Channel, const UScriptStruct* StructType, const void* Payload);

private:
	const void* ReceivedMessagePayloadPtr = nullptr;

	TWeakObjectPtr<UWorld> WorldPtr;
	FGameplayTag ChannelToRegister;
	TWeakObjectPtr<UScriptStruct> MessageStructType = nullptr;
	EMessageRouterMatchRule MessageMatchType = EMessageRouterMatchRule::ExactMatch;

	FMessageRouterListenerHandle ListenerHandle;
};

#undef UE_API
