// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "ExtAbilityCost.h"
#include "ExtAbilityCost_PlayerTagStack.generated.h"

#define UE_API ABILITYEXTENSION_API

/** 从 PlayerState 的标签堆栈扣除能力消耗；需已建立 Controller 和 PlayerState。 */
UCLASS(MinimalAPI)
class UExtAbilityCost_PlayerTagStack : public UExtAbilityCost
{
	GENERATED_BODY()

public:
	UE_API UExtAbilityCost_PlayerTagStack();

	//~UExtAbilityCost interface
	UE_API virtual bool CheckCost(const UExtGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const override;
	UE_API virtual void ApplyCost(const UExtGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	//~End of UExtAbilityCost interface

protected:
	/** 按能力等级计算消耗数量，并向零取整。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Costs)
	FScalableFloat Quantity;

	/** 需要扣除的标签。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Costs)
	FGameplayTag Tag;
};

#undef UE_API
