// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Attributes/ExtAttributeSet.h"
#include "System/ExtAbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ExtAttributeSet)

UExtAttributeSet::UExtAttributeSet()
{
}

UExtAbilitySystemComponent* UExtAttributeSet::GetExtAbilitySystemComponent() const
{
	return Cast<UExtAbilitySystemComponent>(GetOwningAbilitySystemComponent());
}
