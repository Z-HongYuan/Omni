// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "MessageRouterManager.h"
#include "MessageRouterTypes.h"
#include "Engine/CancellableAsyncAction.h"
#include "AsyncAction_ListenForMessage.generated.h"

#define UE_API MESSAGEROUTERS_API


/**
 * 消息到达时传递代理对象和实际通道，专用蓝图节点使用代理读取当前负载。
 * ActualChannel 是发送者的通道；部分匹配时可以是监听通道的子通道。
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAsyncGameplayMessageDelegate, UAsyncAction_ListenForMessage*, ProxyObject, FGameplayTag, ActualChannel);

/** 持续监听消息的异步任务；消息到达时同步转发事件，调用 Cancel 结束监听。 */
UCLASS(MinimalAPI, BlueprintType, meta=(HasDedicatedAsyncNode))
class UAsyncAction_ListenForMessage : public UCancellableAsyncAction
{
	GENERATED_BODY()

public:
	/**
	 * 创建指定通道的持续监听任务；指定 PayloadType 时仅转发相同结构类型的消息。
	 * MatchType 决定是否接收子通道消息；调用 Cancel 撤销监听。
	 */
	UFUNCTION(BlueprintCallable, Category = Messaging, meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true"))
	static UE_API UAsyncAction_ListenForMessage* ListenForMessage(UObject* WorldContextObject, FGameplayTag Channel, UScriptStruct* PayloadType, EMessageRouterMatchRule MatchType = EMessageRouterMatchRule::ExactMatch);

	/**
	 * 在消息回调期间将当前负载复制到 OutPayload，成功时返回 true。
	 * OutPayload 的结构类型必须与任务声明的 PayloadType 完全一致；回调结束后不能读取负载。
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
