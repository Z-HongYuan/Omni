// Copyright © 2026 张鸿源. All Rights Reserved.


#include "System/CustomAbilitySystemGlobals.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CustomAbilitySystemGlobals)

FGameplayEffectContext* UCustomAbilitySystemGlobals::AllocGameplayEffectContext() const
{
	return Super::AllocGameplayEffectContext();
}

FGameplayAbilityActorInfo* UCustomAbilitySystemGlobals::AllocAbilityActorInfo() const
{
	return Super::AllocAbilityActorInfo();
}
