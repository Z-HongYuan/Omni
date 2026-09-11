// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Attributes/CustomAttributeSet.h"
#include "System/CustomAbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CustomAttributeSet)

UCustomAttributeSet::UCustomAttributeSet()
{
}

UCustomAbilitySystemComponent* UCustomAttributeSet::GetCustomAbilitySystemComponent() const
{
	return Cast<UCustomAbilitySystemComponent>(GetOwningAbilitySystemComponent());
}
