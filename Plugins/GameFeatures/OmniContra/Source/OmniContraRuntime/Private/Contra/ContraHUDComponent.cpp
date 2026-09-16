// Copyright © 2026 张鸿源. All Rights Reserved.

#include "Contra/ContraHUDComponent.h"

#include "Contra/ContraHUDWidget.h"
#include "GameFramework/PlayerController.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ContraHUDComponent)

void UContraHUDComponent::BeginPlay()
{
	Super::BeginPlay();
	APlayerController* PC = GetController<APlayerController>();
	if (!PC || !PC->GetLocalPlayer()) return;
	Widget = CreateWidget<UContraHUDWidget>(PC, UContraHUDWidget::StaticClass());
	if (!Widget) return;
	Widget->AddToPlayerScreen();
	Widget->SetPositionInViewport(FVector2D(24, 24));
	Widget->SetDesiredSizeInViewport(FVector2D(680, 180));
	bPreviousMouseCursor = PC->bShowMouseCursor;
	PC->bShowMouseCursor = true;
	PC->SetInputMode(FInputModeGameAndUI().SetHideCursorDuringCapture(false));
}

void UContraHUDComponent::EndPlay(EEndPlayReason::Type Reason)
{
	if (Widget)
	{
		Widget->RemoveFromParent();
		Widget = nullptr;
		if (APlayerController* PC = GetController<APlayerController>())
		{
			PC->bShowMouseCursor = bPreviousMouseCursor;
			PC->SetInputMode(FInputModeGameOnly());
		}
	}
	Super::EndPlay(Reason);
}
