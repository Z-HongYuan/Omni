// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Abilities/ExtGameplayAbility.h"
#include "ExtGameplayAbility_Death.generated.h"

#define UE_API ABILITYEXTENSION_API

/**
 * 由 GameplayEvent.Death 触发的死亡技能基类，通过 AbilitySet 授予，无需输入标签。
 * 沿用 Lyra：服务器发起、每个 Actor 一个实例；取消非豁免技能、禁止取消自身并切换独占阻塞组。
 * 默认开始死亡，结束技能时收尾；具体蓝图负责死亡表现及 EndAbility 时机，本类不自动计时或销毁角色。
 * 对照 Lyra：EndAbility 额外沿用项目的有效性与作用域锁检查；重生及淘汰消息不在本类处理。
 * 项目命名：死亡与 Pawn 注销统一使用 Ability.Behavior.AvoidDeathClear 豁免取消。
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

	// 通知 Health组件的死亡事件
	UFUNCTION(BlueprintCallable, Category = "AbilityExtension|Death")
	UE_API void StartDeath();
	UFUNCTION(BlueprintCallable, Category = "AbilityExtension|Death")
	UE_API void FinishDeath();

	// 关闭后由蓝图自行调用 StartDeath；技能结束时始终尝试 FinishDeath。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AbilityExtension|Death")
	bool bAutoStartDeath = true;
};

#undef UE_API
