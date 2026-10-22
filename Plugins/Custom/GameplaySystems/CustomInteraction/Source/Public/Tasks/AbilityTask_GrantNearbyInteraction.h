// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Abilities/Tasks/AbilityTask.h"
#include "AbilityTask_GrantNearbyInteraction.generated.h"

#define UE_API CUSTOMINTERACTION_API

/**
 * 周期扫描 Avatar 附近的交互目标，并按能力类型缓存已授予的句柄。
 * 由权威端调用；任务销毁时停止扫描，当前不撤销已经授予的能力。
 */
UCLASS(MinimalAPI)
class UAbilityTask_GrantNearbyInteraction : public UAbilityTask
{
	GENERATED_BODY()

public:
	virtual void OnDestroy(bool AbilityEnded) override;

	/** 创建周期球形重叠扫描任务；交互目标的碰撞响应需允许指定通道的查询。 */
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
