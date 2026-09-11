// Copyright © 2026 张鸿源. All Rights Reserved.


#include "MessagingSystem/DialogWidgetDescriptorBase.h"

#include "System/GameUIGameplayTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DialogWidgetDescriptorBase)

UDialogWidgetDescriptorBase* UDialogWidgetDescriptorBase::CreateConfirmationOk(const FText& Title, const FText& Body)
{
	UDialogWidgetDescriptorBase* Descriptor = NewObject<UDialogWidgetDescriptorBase>();
	Descriptor->Title = Title;
	Descriptor->Body = Body;

	FConfirmationDialogAction ConfirmAction;
	ConfirmAction.ActionResult = GameUITags::TAG_GameUI_Dialog_Confirmed;
	ConfirmAction.OptionalDisplayText = NSLOCTEXT("AdvancedMessaging", "Ok", "Ok");

	Descriptor->ButtonActions.Add(ConfirmAction);

	return Descriptor;
}

UDialogWidgetDescriptorBase* UDialogWidgetDescriptorBase::CreateConfirmationOkCancel(const FText& Title, const FText& Body)
{
	UDialogWidgetDescriptorBase* Descriptor = NewObject<UDialogWidgetDescriptorBase>();
	Descriptor->Title = Title;
	Descriptor->Body = Body;

	FConfirmationDialogAction ConfirmAction;
	ConfirmAction.ActionResult = GameUITags::TAG_GameUI_Dialog_Confirmed;
	ConfirmAction.OptionalDisplayText = NSLOCTEXT("AdvancedMessaging", "Ok", "Ok");

	FConfirmationDialogAction CancelAction;
	CancelAction.ActionResult = GameUITags::TAG_GameUI_Dialog_Cancelled;
	CancelAction.OptionalDisplayText = NSLOCTEXT("AdvancedMessaging", "Cancel", "Cancel");

	Descriptor->ButtonActions.Add(ConfirmAction);
	Descriptor->ButtonActions.Add(CancelAction);

	return Descriptor;
}

UDialogWidgetDescriptorBase* UDialogWidgetDescriptorBase::CreateConfirmationYesNo(const FText& Title, const FText& Body)
{
	UDialogWidgetDescriptorBase* Descriptor = NewObject<UDialogWidgetDescriptorBase>();
	Descriptor->Title = Title;
	Descriptor->Body = Body;

	FConfirmationDialogAction ConfirmAction;
	ConfirmAction.ActionResult = GameUITags::TAG_GameUI_Dialog_Confirmed;
	ConfirmAction.OptionalDisplayText = NSLOCTEXT("AdvancedMessaging", "Yes", "Yes");

	FConfirmationDialogAction DeclineAction;
	DeclineAction.ActionResult = GameUITags::TAG_GameUI_Dialog_Declined;
	DeclineAction.OptionalDisplayText = NSLOCTEXT("AdvancedMessaging", "No", "No");

	Descriptor->ButtonActions.Add(ConfirmAction);
	Descriptor->ButtonActions.Add(DeclineAction);

	return Descriptor;
}

UDialogWidgetDescriptorBase* UDialogWidgetDescriptorBase::CreateConfirmationYesNoCancel(const FText& Title, const FText& Body)
{
	UDialogWidgetDescriptorBase* Descriptor = NewObject<UDialogWidgetDescriptorBase>();
	Descriptor->Title = Title;
	Descriptor->Body = Body;

	FConfirmationDialogAction ConfirmAction;
	ConfirmAction.ActionResult = GameUITags::TAG_GameUI_Dialog_Confirmed;
	ConfirmAction.OptionalDisplayText = NSLOCTEXT("AdvancedMessaging", "Yes", "Yes");

	FConfirmationDialogAction DeclineAction;
	DeclineAction.ActionResult = GameUITags::TAG_GameUI_Dialog_Declined;
	DeclineAction.OptionalDisplayText = NSLOCTEXT("AdvancedMessaging", "No", "No");

	FConfirmationDialogAction CancelAction;
	CancelAction.ActionResult = GameUITags::TAG_GameUI_Dialog_Cancelled;
	CancelAction.OptionalDisplayText = NSLOCTEXT("AdvancedMessaging", "Cancel", "Cancel");

	Descriptor->ButtonActions.Add(ConfirmAction);
	Descriptor->ButtonActions.Add(DeclineAction);
	Descriptor->ButtonActions.Add(CancelAction);

	return Descriptor;
}
