// Copyright © 2026 张鸿源. All Rights Reserved.


#include "System/ExtAbilitySystemGlobals.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ExtAbilitySystemGlobals)

FGameplayEffectContext* UExtAbilitySystemGlobals::AllocGameplayEffectContext() const
{
	return Super::AllocGameplayEffectContext();
}

FGameplayAbilityActorInfo* UExtAbilitySystemGlobals::AllocAbilityActorInfo() const
{
	return Super::AllocAbilityActorInfo();
}
