// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "UObject/Object.h"
#include "ExtGameplayAbility.h"
#include "ExtAbilityCost.generated.h"

#define UE_API ABILITYEXTENSION_API

struct FGameplayTagContainer;
struct FGameplayAbilityActorInfo;

/** 由 UExtGameplayAbility 检查并结算的额外消耗，具体资源类型由派生类定义。 */
UCLASS(MinimalAPI, DefaultToInstanced, EditInlineNew, Abstract)
class UExtAbilityCost : public UObject
{
	GENERATED_BODY()

public:
	UExtAbilityCost() { ; }

	/**
	 * 检查资源是否足够；可向 OptionalRelevantTags 追加失败原因，供反馈系统使用。
	 * Ability 和 ActorInfo 由调用方保证有效，OptionalRelevantTags 可以为空。
	 */
	virtual bool CheckCost(const UExtGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
	{
		return true;
	}

	/**
	 * 扣除本次能力消耗；Ability 和 ActorInfo 由调用方保证有效。
	 * 调用方已检查命中条件，派生类无需重复判断 ShouldOnlyApplyCostOnHit。
	 */
	virtual void ApplyCost(const UExtGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
	{
	}

	/** 是否仅在权威端目标数据包含命中结果时结算。 */
	bool ShouldOnlyApplyCostOnHit() const { return bOnlyApplyCostOnHit; }

protected:
	/** 仅在权威端目标数据包含命中结果时结算此项消耗。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Costs)
	bool bOnlyApplyCostOnHit = false;
};

#undef UE_API
