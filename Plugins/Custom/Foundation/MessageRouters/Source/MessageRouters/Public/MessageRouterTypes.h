// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "MessageRouterTypes.generated.h"

// 消息监听器的匹配规则
UENUM(BlueprintType)
enum class EMessageRouterMatchRule : uint8
{
	// 精确匹配只会接收完全相同通道的消息
	//（例如，注册 "A.B" 将匹配 A.B 的广播，但不匹配 A.B.C）
	ExactMatch,

	// 部分匹配将接收以同一通道为根的任何消息
	//（例如，注册 "A.B" 将匹配 A.B 和 A.B.C 的广播）
	PartialMatch
};

/**
 * 用于在注册消息监听器时指定高级行为的结构体
 */
template <typename FMessageStructType>
struct FMessageRouterListenerParams
{
	/** 回调是否接收子通道上的广播，或仅接收完全匹配的通道。 */
	EMessageRouterMatchRule MatchType = EMessageRouterMatchRule::ExactMatch;

	/** 如果绑定，此回调将在指定通道上广播消息时触发。 */
	TFunction<void(FGameplayTag, const FMessageStructType&)> OnMessageReceivedCallback;

	/** 用于将弱成员函数绑定到 OnMessageReceivedCallback 的辅助函数 */
	template <typename TOwner = UObject>
	void SetMessageReceivedCallback(TOwner* Object, void (TOwner::*Function)(FGameplayTag, const FMessageStructType&))
	{
		TWeakObjectPtr<TOwner> WeakObject(Object);
		OnMessageReceivedCallback = [WeakObject, Function](FGameplayTag Channel, const FMessageStructType& Payload)
		{
			if (TOwner* StrongObject = WeakObject.Get())
			{
				(StrongObject->*Function)(Channel, Payload);
			}
		};
	}
};
