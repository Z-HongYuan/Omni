// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "ExtAbilityCost.h"
#include "ExtAbilityCost_PlayerTagStack.generated.h"

#define UE_API ABILITYEXTENSION_API

/**
 * 表示需要消耗玩家状态中一定数量的标签堆叠的成本
 */
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
	/** 该花多少标签（根据能力等级决定） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Costs)
	FScalableFloat Quantity;

	/** 该花哪些标签 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Costs)
	FGameplayTag Tag;
};

#undef UE_API
