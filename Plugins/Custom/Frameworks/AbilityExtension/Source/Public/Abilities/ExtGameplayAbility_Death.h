// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Abilities/ExtGameplayAbility.h"
#include "ExtGameplayAbility_Death.generated.h"

#define UE_API ABILITYEXTENSION_API

/**
 * 由 GameplayEvent.Death 触发的死亡能力，服务器发起，每个 Actor 持有一个实例。
 * 激活时取消未标记 Ability.Behavior.AvoidDeathClear 的其他能力，并切换到独占阻塞组。
 * 默认开始死亡，结束时完成已开始的死亡流程；表现及结束时机由具体蓝图控制。
 */
UCLASS(MinimalAPI, Abstract)
class UExtGameplayAbility_Death : public UExtGameplayAbility
{
	GENERATED_BODY()

public:
	UE_API UExtGameplayAbility_Death(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	UE_API virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	UE_API virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	/** 通知 Health 组件开始死亡。 */
	UFUNCTION(BlueprintCallable, Category = "AbilityExtension|Death")
	UE_API void StartDeath();
	/** 通知 Health 组件完成已经开始的死亡流程。 */
	UFUNCTION(BlueprintCallable, Category = "AbilityExtension|Death")
	UE_API void FinishDeath();

	/** 关闭后由蓝图自行调用 StartDeath；能力结束时始终尝试 FinishDeath。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AbilityExtension|Death")
	bool bAutoStartDeath = true;
};

#undef UE_API
