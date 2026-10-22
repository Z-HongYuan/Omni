// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "CommonButtonBase.h"
#include "GameUIButtonBase.generated.h"

#define UE_API GAMEUI_API

/** 随输入方式和输入动作更新显示文本及样式的按钮基类。 */
UCLASS(MinimalAPI, Abstract)
class UGameUIButtonBase : public UCommonButtonBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="GameUI")
	UE_API void SetButtonText(const FText& InText);

protected:
	// UUserWidget 接口
	UE_API virtual void NativePreConstruct() override;
	// UUserWidget 接口结束

	// UCommonButtonBase 接口
	UE_API virtual void UpdateInputActionWidget() override;
	UE_API virtual void OnInputMethodChanged(ECommonInputType CurrentInputType) override;
	// UCommonButtonBase 接口结束

	UE_API void RefreshButtonText();

	UFUNCTION(BlueprintImplementableEvent)
	UE_API void UpdateButtonText(const FText& InText);

	UFUNCTION(BlueprintImplementableEvent)
	UE_API void UpdateButtonStyle();

private:
	UPROPERTY(EditAnywhere, Category="Button", meta=(InlineEditConditionToggle))
	uint8 bOverride_ButtonText : 1;

	UPROPERTY(EditAnywhere, Category="Button", meta=(Editcondition="bOverride_ButtonText" ))
	FText ButtonText;
};

#undef UE_API
