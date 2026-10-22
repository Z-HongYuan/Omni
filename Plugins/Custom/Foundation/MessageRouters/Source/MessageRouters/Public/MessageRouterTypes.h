// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "MessageRouterTypes.generated.h"

/** 消息通道的匹配规则。 */
UENUM(BlueprintType)
enum class EMessageRouterMatchRule : uint8
{
	/** 仅接收完全相同的通道，例如 A.B 不接收 A.B.C。 */
	ExactMatch,

	/** 接收当前通道及子通道，例如 A.B 同时接收 A.B.C。 */
	PartialMatch
};

/** 注册消息监听时使用的回调和通道匹配参数。 */
template <typename FMessageStructType>
struct FMessageRouterListenerParams
{
	/** 回调是否接收子通道上的广播，或仅接收完全匹配的通道。 */
	EMessageRouterMatchRule MatchType = EMessageRouterMatchRule::ExactMatch;

	/** 如果绑定，此回调将在指定通道上广播消息时触发。 */
	TFunction<void(FGameplayTag, const FMessageStructType&)> OnMessageReceivedCallback;

	/** 绑定对象成员函数，并在调用前检查弱引用，不延长对象生命周期。 */
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
