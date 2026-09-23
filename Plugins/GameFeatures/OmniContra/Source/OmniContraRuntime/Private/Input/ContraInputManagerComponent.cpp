// Copyright © 2026 张鸿源. All Rights Reserved.

#include "Input/ContraInputManagerComponent.h"

#include "GameFramework/Controller.h"
#include "GameplayTagContainer.h"
#include "Input/ExtInputComponent.h"
#include "Input/ExtInputConfig.h"
#include "InputActionValue.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ContraInputManagerComponent)

UContraInputManagerComponent::UContraInputManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UContraInputManagerComponent::BindNativeInputActions(UExtInputComponent* Input, const UExtInputConfig* Config, TArray<uint32>& Handles)
{
	DirectionalInput = FVector2D::ZeroVector;

	const FGameplayTag MoveTag = FGameplayTag::RequestGameplayTag(FName(TEXT("InputTag.Contra.Move")));
	if (const UInputAction* Move = Config->FindNativeInputActionForTag(MoveTag, false))
	{
		Handles.Add(Input->BindAction(Move, ETriggerEvent::Triggered, this, &ThisClass::Input_Move).GetHandle());
		Handles.Add(Input->BindAction(Move, ETriggerEvent::Completed, this, &ThisClass::Input_MoveCompleted).GetHandle());
		Handles.Add(Input->BindAction(Move, ETriggerEvent::Canceled, this, &ThisClass::Input_MoveCompleted).GetHandle());
	}
}

void UContraInputManagerComponent::Input_Move(const FInputActionValue& Value)
{
	DirectionalInput = Value.Get<FVector2D>();

	if (APawn* Pawn = GetPawn<APawn>())
	{
		Pawn->AddMovementInput(FVector::ForwardVector, DirectionalInput.X);
		if (!FMath::IsNearlyZero(DirectionalInput.X))
		{
			if (AController* Controller = Pawn->GetController())
			{
				FRotator FacingRotation = Controller->GetControlRotation();
				FacingRotation.Yaw = DirectionalInput.X > 0.0f ? 0.0f : 180.0f;
				// Pawn 使用控制器 Yaw，朝向随 CharacterMovement 的移动数据同步。
				Controller->SetControlRotation(FacingRotation);
			}
		}
	}
}

void UContraInputManagerComponent::Input_MoveCompleted(const FInputActionValue&)
{
	DirectionalInput = FVector2D::ZeroVector;
}
