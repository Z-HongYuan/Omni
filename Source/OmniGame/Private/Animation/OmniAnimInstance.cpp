// Copyright © 2026 张鸿源. All Rights Reserved.

#include "Animation/OmniAnimInstance.h"

#include "Character/OmniCMC.h"
#include "GameFramework/Character.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(OmniAnimInstance)

UOmniAnimInstance::UOmniAnimInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UOmniCMC* UOmniAnimInstance::GetOmniCMC() const
{
	if (const ACharacter* Character = Cast<ACharacter>(GetOwningActor()))
	{
		return Cast<UOmniCMC>(Character->GetCharacterMovement());
	}
	return nullptr;
}
