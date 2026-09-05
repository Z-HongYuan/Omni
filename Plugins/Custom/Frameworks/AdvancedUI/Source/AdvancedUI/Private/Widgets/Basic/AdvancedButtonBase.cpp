// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Widgets/Basic/AdvancedButtonBase.h"

#include "CommonActionWidget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AdvancedButtonBase)

void UAdvancedButtonBase::SetButtonText(const FText& InText)
{
	bOverride_ButtonText = InText.IsEmpty();
	ButtonText = InText;
	RefreshButtonText();
}

void UAdvancedButtonBase::NativePreConstruct()
{
	Super::NativePreConstruct();

	UpdateButtonStyle();
	RefreshButtonText();
}

void UAdvancedButtonBase::UpdateInputActionWidget()
{
	Super::UpdateInputActionWidget();

	UpdateButtonStyle();
	RefreshButtonText();
}

void UAdvancedButtonBase::OnInputMethodChanged(ECommonInputType CurrentInputType)
{
	Super::OnInputMethodChanged(CurrentInputType);

	UpdateButtonStyle();
}

void UAdvancedButtonBase::RefreshButtonText()
{
	if (bOverride_ButtonText || ButtonText.IsEmpty())
	{
		if (InputActionWidget)
		{
			const FText ActionDisplayText = InputActionWidget->GetDisplayText();
			if (!ActionDisplayText.IsEmpty())
			{
				UpdateButtonText(ActionDisplayText);
				return;
			}
		}
	}

	UpdateButtonText(ButtonText);
}
