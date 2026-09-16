// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Abilities/ExtGameplayAbility.h"
#include "ContraGameplayAbility_Fire.generated.h"

/** 首轮持续射击能力：预测激活与松开，命中和伤害由服务端武器组件负责。 */
UCLASS()
class OMNICONTRARUNTIME_API UContraGameplayAbility_Fire : public UExtGameplayAbility
{
	GENERATED_BODY()

public:
	UContraGameplayAbility_Fire(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual void ActivateAbility(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                             FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                        FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

private:
	void Fire();
	UFUNCTION()
	void Released(float TimeHeld);
	FTimerHandle FireTimer;
};
