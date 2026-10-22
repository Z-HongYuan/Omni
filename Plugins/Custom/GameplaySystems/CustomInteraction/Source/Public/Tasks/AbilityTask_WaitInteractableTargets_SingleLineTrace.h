// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "AbilityTask_WaitInteractableTargets.h"
#include "Data/InteractionQuery.h"
#include "AbilityTask_WaitInteractableTargets_SingleLineTrace.generated.h"

#define UE_API CUSTOMINTERACTION_API

/** 周期执行线追踪，在可交互选项变化时通知调用方。 */
UCLASS(MinimalAPI)
class UAbilityTask_WaitInteractableTargets_SingleLineTrace : public UAbilityTask_WaitInteractableTargets
{
	GENERATED_BODY()

public:
	virtual void OnDestroy(bool AbilityEnded) override;

protected:
	virtual void Activate() override;

	/** 创建周期线追踪任务；任务销毁时清除扫描定时器。 */
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
	/** 追踪当前瞄准方向，并更新命中目标的可用交互选项。 */
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
