// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "CommonActivatableWidget.h"
#include "MessagingManager.h"
#include "Engine/DataTable.h"
#include "DialogWidgetBase.generated.h"

#define UE_API GAMEUI_API

class UCommonBorder;
class UDynamicEntryBox;
class UCommonRichTextBlock;
class UCommonTextBlock;
class UDialogWidgetDescriptorBase;

/** 根据描述创建对话选项，并在关闭时回调选择结果。 */
UCLASS(MinimalAPI, Abstract)
class UDialogWidgetBase : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UE_API UDialogWidgetBase() { ; };

	/** 设置标题、正文和按钮，并保存结果回调；Descriptor 须有效。 */
	UE_API virtual void SetupDialog(UDialogWidgetDescriptorBase* Descriptor, FDialogMessagingResultDelegate ResultCallback);

	/** 主动关闭对话框，以 Killed 结果通知调用方。 */
	UE_API virtual void KillDialog();

#if WITH_EDITOR
	virtual void ValidateCompiledDefaults(IWidgetCompilerLog& CompileLog) const override;
#endif

protected:
	virtual void NativeOnInitialized() override;

	/** 停用对话框并执行结果回调。 */
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
