// Copyright © 2026 张鸿源. All Rights Reserved.

#include "MessagingSystem/DialogWidgetBase.h"

#include "CommonBorder.h"
#include "CommonRichTextBlock.h"
#include "CommonTextBlock.h"
#include "ICommonInputModule.h"
#include "Components/DynamicEntryBox.h"
#include "LogGameUI.h"
#include "System/GameUIGameplayTags.h"
#include "Widgets/Basic/GameUIButtonBase.h"

#if WITH_EDITOR
#include "CommonInputSettings.h"
#include "Editor/WidgetCompilerLog.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(DialogWidgetBase)

void UDialogWidgetBase::SetupDialog(UDialogWidgetDescriptorBase* Descriptor, FDialogMessagingResultDelegate ResultCallback)
{
	Text_Title->SetText(Descriptor->Title);
	RichText_Description->SetText(Descriptor->Body);

	EntryBox_Buttons->Reset<UGameUIButtonBase>([](const UGameUIButtonBase& Button)
	{
		Button.OnClicked().Clear();
	});

	for (const FConfirmationDialogAction& Action : Descriptor->ButtonActions)
	{
		FDataTableRowHandle ActionRow;

		if (Action.ActionResult.MatchesTagExact(GameUITags::TAG_GameUI_Dialog_Confirmed))
		{
			ActionRow = ICommonInputModule::GetSettings().GetDefaultClickAction();
		}
		else if (Action.ActionResult.MatchesTagExact(GameUITags::TAG_GameUI_Dialog_Declined))
		{
			ActionRow = ICommonInputModule::GetSettings().GetDefaultBackAction();
		}
		else if (Action.ActionResult.MatchesTagExact(GameUITags::TAG_GameUI_Dialog_Cancelled))
		{
			ActionRow = CancelAction;
		}
		else
		{
			// 未匹配到预设结果的按钮使用默认点击动作,避免按钮没有任何触发输入动作
			UE_LOG(LogGameUI, Warning, TEXT("UDialogWidgetBase::SetupDialog: Action result [%s] has no predefined input action, falling back to default click action"), *Action.ActionResult.ToString());
			ActionRow = ICommonInputModule::GetSettings().GetDefaultClickAction();
		}

		UGameUIButtonBase* Button = EntryBox_Buttons->CreateEntry<UGameUIButtonBase>();
		Button->SetTriggeringInputAction(ActionRow);
		Button->OnClicked().AddUObject(this, &ThisClass::CloseConfirmationWindow, Action.ActionResult);
		Button->SetButtonText(Action.OptionalDisplayText);
	}

	OnResultCallback = ResultCallback;
}

void UDialogWidgetBase::KillDialog()
{
	// 明确关闭对话框(无用户输入),以 Killed 结果通知调用方
	CloseConfirmationWindow(GameUITags::TAG_GameUI_Dialog_Killed);
}

#if WITH_EDITOR
void UDialogWidgetBase::ValidateCompiledDefaults(IWidgetCompilerLog& CompileLog) const
{
	Super::ValidateCompiledDefaults(CompileLog);
	if (CancelAction.IsNull())
	{
		CompileLog.Error(FText::Format(FText::FromString(TEXT("{0} has unset property: CancelAction.")), FText::FromString(GetName())));
	}
}
#endif

void UDialogWidgetBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	Border_TapToCloseZone->OnMouseButtonDownEvent.BindDynamic(this, &UDialogWidgetBase::HandleTapToCloseZoneMouseButtonDown);
}

void UDialogWidgetBase::CloseConfirmationWindow(FGameplayTag Result)
{
	DeactivateWidget();
	OnResultCallback.ExecuteIfBound(Result);
}

FEventReply UDialogWidgetBase::HandleTapToCloseZoneMouseButtonDown(FGeometry MyGeometry, const FPointerEvent& MouseEvent)
{
	FEventReply Reply;
	Reply.NativeReply = FReply::Unhandled();

	if (MouseEvent.IsTouchEvent() || MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		CloseConfirmationWindow(GameUITags::TAG_GameUI_Dialog_Declined);
		Reply.NativeReply = FReply::Handled();
	}

	return Reply;
}
