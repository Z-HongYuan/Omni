// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Abilities/ExtGameplayAbility.h"
#include "OmniGameplayAbility_Jump.generated.h"

#define UE_API OMNIGAME_API

/**
 * 最小跳跃能力：按下开始跳跃，松开或取消时停止，直接通过 AbilitySet 授予。
 * 对照 Lyra：保留每角色实例、本地预测与 EndAbility 清理；激活流程放在 C++。
 * 后续按需添加 Jumping 状态任务、能力标签及触屏 UI，本类不依赖蓝图编排。
 */
UCLASS(MinimalAPI)
class UOmniGameplayAbility_Jump : public UExtGameplayAbility
{
	GENERATED_BODY()

public:
	UE_API UOmniGameplayAbility_Jump(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	UE_API virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags,
	                                       FGameplayTagContainer* OptionalRelevantTags) const override;
	UE_API virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	UE_API virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

private:
	UFUNCTION()
	void OnInputReleased(float TimeHeld);
};

#undef UE_API
