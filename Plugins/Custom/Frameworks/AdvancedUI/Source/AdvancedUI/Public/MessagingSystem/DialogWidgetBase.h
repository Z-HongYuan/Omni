// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "CommonActivatableWidget.h"
#include "MessagingManager.h"
#include "Engine/DataTable.h"
#include "DialogWidgetBase.generated.h"

#define UE_API ADVANCEDUI_API

class UCommonBorder;
class UDynamicEntryBox;
class UCommonRichTextBlock;
class UCommonTextBlock;
class UDialogWidgetDescriptorBase;

/**
 * 对话系统使用的控件基类, 提供了事件调用
 */
UCLASS(MinimalAPI, Abstract)
class UDialogWidgetBase : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UE_API UDialogWidgetBase() { ; };

	// 设置对话信息, 并绑定委托
	UE_API virtual void SetupDialog(UDialogWidgetDescriptorBase* Descriptor, FDialogMessagingResultDelegate ResultCallback);

	UE_API virtual void KillDialog();

#if WITH_EDITOR
	virtual void ValidateCompiledDefaults(IWidgetCompilerLog& CompileLog) const override;
#endif

protected:
	virtual void NativeOnInitialized() override;

	// 关闭对话之后返回的结果
	virtual void CloseConfirmationWindow(FGameplayTag Result);

private:
	UFUNCTION()
	FEventReply HandleTapToCloseZoneMouseButtonDown(FGeometry MyGeometry, const FPointerEvent& MouseEvent);

	FDialogMessagingResultDelegate OnResultCallback;

	UPROPERTY(Meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> Text_Title;

	UPROPERTY(Meta = (BindWidget))
	TObjectPtr<UCommonRichTextBlock> RichText_Description;

	UPROPERTY(Meta = (BindWidget))
	TObjectPtr<UDynamicEntryBox> EntryBox_Buttons;

	UPROPERTY(Meta = (BindWidget))
	TObjectPtr<UCommonBorder> Border_TapToCloseZone;

	UPROPERTY(EditDefaultsOnly, meta = (RowType = "/Script/CommonUI.CommonInputActionDataBase"))
	FDataTableRowHandle CancelAction;
};
#undef UE_API
