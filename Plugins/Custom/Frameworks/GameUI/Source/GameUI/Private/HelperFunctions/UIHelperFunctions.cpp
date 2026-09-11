// Copyright © 2026 张鸿源. All Rights Reserved.


#include "HelperFunctions/UIHelperFunctions.h"
#include "CommonActivatableWidget.h"
#include "CommonInputSubsystem.h"
#include "GameplayTagContainer.h"
#include "Engine/GameInstance.h"
#include "System/GameUIManager.h"
#include "System/GameUIPolicy.h"
#include "Widgets/GameUIRootWidget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UIHelperFunctions)

int32 UUIHelperFunctions::InputSuspensions = 0;

ECommonInputType UUIHelperFunctions::GetOwningPlayerInputType(const UUserWidget* WidgetContextObject)
{
	if (WidgetContextObject)
	{
		if (const UCommonInputSubsystem* InputSubsystem = UCommonInputSubsystem::Get(WidgetContextObject->GetOwningLocalPlayer()))
		{
			return InputSubsystem->GetCurrentInputType();
		}
	}

	return ECommonInputType::Count;
}

bool UUIHelperFunctions::IsOwningPlayerUsingTouch(const UUserWidget* WidgetContextObject)
{
	if (WidgetContextObject)
	{
		if (const UCommonInputSubsystem* InputSubsystem = UCommonInputSubsystem::Get(WidgetContextObject->GetOwningLocalPlayer()))
		{
			return InputSubsystem->GetCurrentInputType() == ECommonInputType::Touch;
		}
	}
	return false;
}

bool UUIHelperFunctions::IsOwningPlayerUsingGamepad(const UUserWidget* WidgetContextObject)
{
	if (WidgetContextObject)
	{
		if (const UCommonInputSubsystem* InputSubsystem = UCommonInputSubsystem::Get(WidgetContextObject->GetOwningLocalPlayer()))
		{
			return InputSubsystem->GetCurrentInputType() == ECommonInputType::Gamepad;
		}
	}
	return false;
}

UCommonActivatableWidget* UUIHelperFunctions::PushWidgetToLayerForPlayer(const ULocalPlayer* LocalPlayer, FGameplayTag LayerName, TSubclassOf<UCommonActivatableWidget> WidgetClass)
{
	if (!ensure(LocalPlayer) || !ensure(WidgetClass != nullptr))
	{
		return nullptr;
	}

	if (UGameUIManager* GameUIManager = LocalPlayer->GetGameInstance()->GetSubsystem<UGameUIManager>())
	{
		if (UGameUIPolicy* Policy = GameUIManager->GetCurrentUIPolicy())
		{
			if (UGameUIRootWidget* RootLayout = Policy->GetRootLayoutWidget(LocalPlayer))
			{
				return RootLayout->PushWidgetToLayerStack(LayerName, WidgetClass);
			}
		}
	}

	return nullptr;
}

void UUIHelperFunctions::PushSoftWidgetToLayerForPlayer(const ULocalPlayer* LocalPlayer, FGameplayTag LayerName, TSoftClassPtr<UCommonActivatableWidget> WidgetClass)
{
	if (!ensure(LocalPlayer) || !ensure(!WidgetClass.IsNull()))
	{
		return;
	}

	if (UGameUIManager* GameUIManager = LocalPlayer->GetGameInstance()->GetSubsystem<UGameUIManager>())
	{
		if (UGameUIPolicy* Policy = GameUIManager->GetCurrentUIPolicy())
		{
			if (UGameUIRootWidget* RootLayout = Policy->GetRootLayoutWidget(LocalPlayer))
			{
				RootLayout->PushWidgetToLayerStackAsync(LayerName, true, WidgetClass);
			}
		}
	}
}

void UUIHelperFunctions::RemoveWidgetFromLayer(UCommonActivatableWidget* ActivatableWidget)
{
	if (!ActivatableWidget)
	{
		return;
	}

	if (const ULocalPlayer* LocalPlayer = ActivatableWidget->GetOwningLocalPlayer())
	{
		if (const UGameUIManager* GameUIManager = LocalPlayer->GetGameInstance()->GetSubsystem<UGameUIManager>())
		{
			if (const UGameUIPolicy* Policy = GameUIManager->GetCurrentUIPolicy())
			{
				if (UGameUIRootWidget* RootLayout = Policy->GetRootLayoutWidget(LocalPlayer))
				{
					RootLayout->FindAndRemoveWidgetFromLayer(ActivatableWidget);
				}
			}
		}
	}
}

ULocalPlayer* UUIHelperFunctions::GetLocalPlayerFromController(APlayerController* PlayerController)
{
	if (PlayerController) return PlayerController->GetLocalPlayer();

	return nullptr;
}

FName UUIHelperFunctions::SuspendInputForPlayer(APlayerController* PlayerController, FName SuspendReason)
{
	return SuspendInputForPlayer(PlayerController ? PlayerController->GetLocalPlayer() : nullptr, SuspendReason);
}

FName UUIHelperFunctions::SuspendInputForPlayer(const ULocalPlayer* LocalPlayer, FName SuspendReason)
{
	if (UCommonInputSubsystem* CommonInputSubsystem = UCommonInputSubsystem::Get(LocalPlayer))
	{
		InputSuspensions++;
		FName SuspendToken = SuspendReason;
		SuspendToken.SetNumber(InputSuspensions);

		CommonInputSubsystem->SetInputTypeFilter(ECommonInputType::MouseAndKeyboard, SuspendToken, true);
		CommonInputSubsystem->SetInputTypeFilter(ECommonInputType::Gamepad, SuspendToken, true);
		CommonInputSubsystem->SetInputTypeFilter(ECommonInputType::Touch, SuspendToken, true);

		return SuspendToken;
	}

	return NAME_None;
}

void UUIHelperFunctions::ResumeInputForPlayer(APlayerController* PlayerController, FName SuspendToken)
{
	ResumeInputForPlayer(PlayerController ? PlayerController->GetLocalPlayer() : nullptr, SuspendToken);
}

void UUIHelperFunctions::ResumeInputForPlayer(const ULocalPlayer* LocalPlayer, FName SuspendToken)
{
	if (SuspendToken == NAME_None) return;

	if (UCommonInputSubsystem* CommonInputSubsystem = UCommonInputSubsystem::Get(LocalPlayer))
	{
		CommonInputSubsystem->SetInputTypeFilter(ECommonInputType::MouseAndKeyboard, SuspendToken, false);
		CommonInputSubsystem->SetInputTypeFilter(ECommonInputType::Gamepad, SuspendToken, false);
		CommonInputSubsystem->SetInputTypeFilter(ECommonInputType::Touch, SuspendToken, false);
	}
}
