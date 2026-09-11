// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Abilities/Tasks/AbilityTask.h"
#include "AbilityTask_GrantNearbyInteraction.generated.h"

#define UE_API CUSTOMINTERACTION_API

/**
 * 
 */
UCLASS(MinimalAPI)
class UAbilityTask_GrantNearbyInteraction : public UAbilityTask
{
	GENERATED_BODY()

public:
	virtual void OnDestroy(bool AbilityEnded) override;

	/** 等到出现重叠时再说。这需要更详细地完善，以便我们能够明确游戏特定的碰撞要求
	 * 可交互物体必须 Block 这个 Interaction 通道。*/
	UFUNCTION(BlueprintCallable, Category="Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UAbilityTask_GrantNearbyInteraction* GrantAbilitiesForNearbyInteractors(UGameplayAbility* OwningAbility, ECollisionChannel TraceChannel, float InteractionScanRange, float InteractionScanRate);

protected:
	virtual void Activate() override;

private:
	void QueryInteractables();

	float InteractionScanRange = 100.f;
	float InteractionScanRate = 0.100f;

	ECollisionChannel TraceChannel;

	FTimerHandle QueryTimerHandle;

	TMap<FObjectKey, FGameplayAbilitySpecHandle> InteractionAbilityCache;
};

#undef UE_API
