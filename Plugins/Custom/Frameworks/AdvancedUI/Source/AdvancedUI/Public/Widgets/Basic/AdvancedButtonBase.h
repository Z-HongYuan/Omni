// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "CommonButtonBase.h"
#include "AdvancedButtonBase.generated.h"

#define UE_API ADVANCEDUI_API

/**
 * 最基础的按钮按键
 */
UCLASS(MinimalAPI, Abstract)
class UAdvancedButtonBase : public UCommonButtonBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="AdvancedUI")
	void SetButtonText(const FText& InText);

protected:
	// UUserWidget interface
	virtual void NativePreConstruct() override;
	// End of UUserWidget interface

	// UCommonButtonBase interface
	virtual void UpdateInputActionWidget() override;
	virtual void OnInputMethodChanged(ECommonInputType CurrentInputType) override;
	// End of UCommonButtonBase interface

	void RefreshButtonText();

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateButtonText(const FText& InText);

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateButtonStyle();

private:
	UPROPERTY(EditAnywhere, Category="Button", meta=(InlineEditConditionToggle))
	uint8 bOverride_ButtonText : 1;

	UPROPERTY(EditAnywhere, Category="Button", meta=(Editcondition="bOverride_ButtonText" ))
	FText ButtonText;
};


#undef UE_API
