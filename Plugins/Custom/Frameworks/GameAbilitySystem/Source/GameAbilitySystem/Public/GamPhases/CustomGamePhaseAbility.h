// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Abilities/CustomGameplayAbility.h"
#include "CustomGamePhaseAbility.generated.h"

#define UE_API GAMEABILITYSYSTEM_API

/**
 * 定义此游戏阶段能力所属的游戏阶段。例如，
	如果你的游戏阶段是 GamePhase.RoundStart，那么它将取消所有同级阶段。
	所以如果你有一个像 GamePhase.WaitingToStart 这样的活动阶段正在启动
	RoundStart的能力部分将结束WaitingToStart。不过，要实现嵌套行为
	你也可以嵌套阶段。例如，GamePhase.Playing.NormalPlay 就是一个子阶段
	属于父游戏阶段 GamePhase.Playing，因此将子阶段更改为 GamePhase.Playing.SuddenDeath，
	将停止与 GamePhase.Playing.* 相关的任何能力，但不会结束任何能力
	与 GamePhase.Playing 阶段相关联。
 */
UCLASS(MinimalAPI)
class UCustomGamePhaseAbility : public UCustomGameplayAbility
{
	GENERATED_BODY()

public:
	UE_API UCustomGamePhaseAbility(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UE_API const FGameplayTag& GetGamePhaseTag() const { return GamePhaseTag; }

#if WITH_EDITOR
	UE_API virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

protected:
	UE_API virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	UE_API virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	// 定义此游戏阶段能力所属的游戏阶段。例如，
	// 如果你的游戏阶段是 GamePhase.RoundStart，那么它将取消所有同级阶段。
	// 所以如果你有一个像 GamePhase.WaitingToStart 这样的活动阶段正在启动
	// RoundStart的能力部分将结束WaitingToStart。不过，要实现嵌套行为
	// 你也可以嵌套阶段。例如，GamePhase.Playing.NormalPlay 就是一个子阶段
	// 属于父游戏阶段 GamePhase.Playing，因此将子阶段更改为 GamePhase.Playing.SuddenDeath，
	// 将停止与 GamePhase.Playing.* 相关的任何能力，但不会结束任何能力
	// 与 GamePhase.Playing 阶段相关联。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom|Game Phase")
	FGameplayTag GamePhaseTag;
};

#undef UE_API
