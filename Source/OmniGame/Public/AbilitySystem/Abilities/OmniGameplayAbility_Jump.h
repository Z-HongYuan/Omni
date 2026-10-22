// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Abilities/ExtGameplayAbility.h"
#include "OmniGameplayAbility_Jump.generated.h"

#define UE_API OMNIGAME_API

/**
 * 通过 AbilitySet 授予的跳跃能力，按下时跳跃，松开或取消时停止。
 * 使用 InstancedPerActor 实例化策略和本地预测，并在 EndAbility 中清理跳跃状态。
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
