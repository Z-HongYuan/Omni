// Copyright © 2026 张鸿源. All Rights Reserved.

#include "Contra/ContraInputManagerComponent.h"

#include "GameFramework/PlayerController.h"
#include "Input/ExtInputComponent.h"
#include "InputActionValue.h"
#include "OmniGame/OmniTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ContraInputManagerComponent)

UContraInputManagerComponent::UContraInputManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UContraInputManagerComponent::BindNativeInputActions(UExtInputComponent* Input, const UExtInputConfig* Config, TArray<uint32>& Handles)
{
	if (const UInputAction* Action = Config->FindNativeInputActionForTag(OmniTags::TAG_InputTag_Move, true))
	{
		Handles.Add(Input->BindAction(Action, ETriggerEvent::Triggered, this, &ThisClass::Move).GetHandle());
	}
	if (const UInputAction* Action = Config->FindNativeInputActionForTag(OmniTags::TAG_InputTag_Look_Mouse, true))
	{
		Handles.Add(Input->BindAction(Action, ETriggerEvent::Triggered, this, &ThisClass::AimMouse).GetHandle());
	}
	const FGameplayTag StickTag = FGameplayTag::RequestGameplayTag(TEXT("InputTag.Contra.Aim.Stick"));
	if (const UInputAction* Action = Config->FindNativeInputActionForTag(StickTag, true))
	{
		Handles.Add(Input->BindAction(Action, ETriggerEvent::Triggered, this, &ThisClass::AimStick).GetHandle());
	}
}

void UContraInputManagerComponent::Move(const FInputActionValue& Value)
{
	if (APawn* Pawn = GetPawn<APawn>()) Pawn->AddMovementInput(FVector::ForwardVector, Value.Get<FVector2D>().X);
}

void UContraInputManagerComponent::SetAim(FVector2D Direction)
{
	if (Direction.SizeSquared() < 0.04) return;
	APlayerController* Controller = GetController<APlayerController>();
	if (!Controller || !Controller->IsLocalController()) return;

	const double Step = UE_DOUBLE_PI / 4.0;
	const double Angle = FMath::RoundToDouble(FMath::Atan2(Direction.Y, Direction.X) / Step) * Step;
	// ControlRotation 随角色移动协议送达服务器，枪口仍由服务器自己的 Pawn 计算。
	Controller->SetControlRotation(FVector(FMath::Cos(Angle), 0.0, FMath::Sin(Angle)).Rotation());
}

void UContraInputManagerComponent::AimStick(const FInputActionValue& Value)
{
	const FVector2D Direction = Value.Get<FVector2D>();
	if (Direction.SizeSquared() < 0.04) return;
	bUseStickAim = true;
	SetAim(Direction);
}

void UContraInputManagerComponent::AimMouse(const FInputActionValue& Value)
{
	if (!Value.Get<FVector2D>().IsNearlyZero()) bUseStickAim = false;
}

void UContraInputManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	const APawn* Pawn = GetPawn<APawn>();
	APlayerController* Controller = GetController<APlayerController>();
	if (bUseStickAim || !Pawn || !Controller || !Controller->GetLocalPlayer()) return;

	FVector Origin, Direction;
	if (!Controller->DeprojectMousePositionToWorld(Origin, Direction) || FMath::IsNearlyZero(Direction.Y)) return;
	const double Distance = (Pawn->GetActorLocation().Y - Origin.Y) / Direction.Y;
	if (Distance <= 0.0) return;
	const FVector Aim = Origin + Direction * Distance - Pawn->GetActorLocation();
	SetAim(FVector2D(Aim.X, Aim.Z));
}
