// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "AbilityTask_WaitInteractableTargets.h"
#include "Data/InteractionQuery.h"
#include "AbilityTask_WaitInteractableTargets_SingleLineTrace.generated.h"

#define UE_API CUSTOMINTERACTION_API

/**
 * 等待交互目标,使用线追踪
 */
UCLASS(MinimalAPI)
class UAbilityTask_WaitInteractableTargets_SingleLineTrace : public UAbilityTask_WaitInteractableTargets
{
	GENERATED_BODY()

public:
	virtual void OnDestroy(bool AbilityEnded) override;

protected:
	virtual void Activate() override;

	/** 等我们追踪到新的互动对象。这个任务会自动循环。 */
	UFUNCTION(BlueprintCallable, Category="Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UAbilityTask_WaitInteractableTargets_SingleLineTrace* WaitForInteractableTargets_SingleLineTrace(
		UGameplayAbility* OwningAbility,
		FInteractionQuery InteractionQuery,
		FCollisionProfileName TraceProfile,
		FGameplayAbilityTargetingLocationInfo StartLocation,
		float InteractionScanRange = 100,
		float InteractionScanRate = 0.100,
		bool bShowDebug = false);

private:
	// 执行追踪
	void PerformTrace();

	UPROPERTY()
	FInteractionQuery InteractionQuery;

	UPROPERTY()
	FGameplayAbilityTargetingLocationInfo StartLocation;

	float InteractionScanRange = 100.f;
	float InteractionScanRate = 0.100f;
	bool bShowDebug = false;

	FTimerHandle TimerHandle;
};

#undef UE_API
