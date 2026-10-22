// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "AbilitySystemGlobals.h"
#include "ExtAbilitySystemGlobals.generated.h"

#define UE_API ABILITYEXTENSION_API

/** 提供效果上下文和 ActorInfo 的分配扩展入口，当前均沿用父类实现。 */
UCLASS(MinimalAPI)
class UExtAbilitySystemGlobals : public UAbilitySystemGlobals
{
	GENERATED_BODY()

public:
	//~UAbilitySystemGlobals interface
	UE_API virtual FGameplayEffectContext* AllocGameplayEffectContext() const override;
	UE_API virtual FGameplayAbilityActorInfo* AllocAbilityActorInfo() const override;
	//~End of UAbilitySystemGlobals interface
};

#undef UE_API
