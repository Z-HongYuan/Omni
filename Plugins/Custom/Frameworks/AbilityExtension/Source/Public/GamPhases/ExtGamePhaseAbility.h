// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Abilities/ExtGameplayAbility.h"
#include "ExtGamePhaseAbility.generated.h"

#define UE_API ABILITYEXTENSION_API

/**
 * 由服务器驱动的游戏阶段能力，激活和结束时通知阶段管理器。
 * 新阶段保留相同标签及其父阶段，取消其他活跃阶段，支持父子阶段共存。
 */
UCLASS(MinimalAPI)
class UExtGamePhaseAbility : public UExtGameplayAbility
{
	GENERATED_BODY()

public:
	UE_API UExtGamePhaseAbility(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UE_API const FGameplayTag& GetGamePhaseTag() const { return GamePhaseTag; }

#if WITH_EDITOR
	UE_API virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

protected:
	UE_API virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	UE_API virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	/** 阶段标签，通过标签层级定义父子关系；同一标签可以对应多个阶段能力。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AbilityExtension|Game Phase")
	FGameplayTag GamePhaseTag;
};

#undef UE_API
