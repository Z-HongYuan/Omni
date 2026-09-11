// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "AbilitySystemGlobals.h"
#include "ExtAbilitySystemGlobals.generated.h"

#define UE_API ABILITYEXTENSION_API

/**
 * 配置ASC系统的全局参数
 * 应该针对于不同的项目进行配置和重载,使用需要的 FGameplayEffectContext 和 FGameplayAbilityActorInfo
 */
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
