// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "CustomAbilityCost.h"
#include "CustomAbilityCost_PlayerTagStack.generated.h"

#define UE_API GAMEABILITYSYSTEM_API

/**
 * 表示需要消耗玩家状态中一定数量的标签堆叠的成本
 */
UCLASS(MinimalAPI)
class UCustomAbilityCost_PlayerTagStack : public UCustomAbilityCost
{
	GENERATED_BODY()

public:
	UE_API UCustomAbilityCost_PlayerTagStack();

	//~UCustomAbilityCost interface
	UE_API virtual bool CheckCost(const UCustomGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const override;
	UE_API virtual void ApplyCost(const UCustomGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	//~End of UCustomAbilityCost interface

protected:
	/** 该花多少标签（根据能力等级决定） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Costs)
	FScalableFloat Quantity;

	/** 该花哪些标签 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Costs)
	FGameplayTag Tag;
};

#undef UE_API
