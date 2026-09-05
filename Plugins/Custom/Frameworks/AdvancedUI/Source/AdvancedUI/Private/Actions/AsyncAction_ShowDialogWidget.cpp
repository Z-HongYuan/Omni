// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Actions/AsyncAction_ShowDialogWidget.h"

#include "Blueprint/UserWidget.h"
#include "Engine/GameInstance.h"
#include "MessagingSystem/DialogWidgetDescriptorBase.h"
#include "MessagingSystem/MessagingManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AsyncAction_ShowDialogWidget)

UAsyncAction_ShowDialogWidget* UAsyncAction_ShowDialogWidget::ShowDialogYesNo(UObject* InWorldContextObject, FGameplayTag InWidgetTag, FText Title, FText Message)
{
	UAsyncAction_ShowDialogWidget* Action = NewObject<UAsyncAction_ShowDialogWidget>();
	Action->WorldContextObject = InWorldContextObject;
	Action->Descriptor = UDialogWidgetDescriptorBase::CreateConfirmationYesNo(Title, Message);
	Action->WidgetTag = InWidgetTag;
	Action->RegisterWithGameInstance(InWorldContextObject);

	return Action;
}

UAsyncAction_ShowDialogWidget* UAsyncAction_ShowDialogWidget::ShowDialogOkCancel(UObject* InWorldContextObject, FGameplayTag InWidgetTag, FText Title, FText Message)
{
	UAsyncAction_ShowDialogWidget* Action = NewObject<UAsyncAction_ShowDialogWidget>();
	Action->WorldContextObject = InWorldContextObject;
	Action->Descriptor = UDialogWidgetDescriptorBase::CreateConfirmationOkCancel(Title, Message);
	Action->WidgetTag = InWidgetTag;
	Action->RegisterWithGameInstance(InWorldContextObject);

	return Action;
}

UAsyncAction_ShowDialogWidget* UAsyncAction_ShowDialogWidget::ShowDialogCustom(UObject* InWorldContextObject, FGameplayTag InWidgetTag, UDialogWidgetDescriptorBase* Descriptor)
{
	UAsyncAction_ShowDialogWidget* Action = NewObject<UAsyncAction_ShowDialogWidget>();
	Action->WorldContextObject = InWorldContextObject;
	Action->Descriptor = Descriptor;
	Action->WidgetTag = InWidgetTag;
	Action->RegisterWithGameInstance(InWorldContextObject);

	return Action;
}

void UAsyncAction_ShowDialogWidget::Activate()
{
	// 通过各个位置的调用点获取到本地玩家
	if (WorldContextObject && !TargetLocalPlayer)
	{
		if (UUserWidget* UserWidget = Cast<UUserWidget>(WorldContextObject))
		{
			TargetLocalPlayer = UserWidget->GetOwningLocalPlayer<ULocalPlayer>();
		}
		else if (APlayerController* PC = Cast<APlayerController>(WorldContextObject))
		{
			TargetLocalPlayer = PC->GetLocalPlayer();
		}
		else if (UWorld* World = WorldContextObject->GetWorld())
		{
			if (UGameInstance* GameInstance = World->GetGameInstance<UGameInstance>())
			{
				TargetLocalPlayer = GameInstance->GetPrimaryPlayerController(false)->GetLocalPlayer();
			}
		}
	}

	//如果有本地玩家就调用对话框
	if (TargetLocalPlayer)
	{
		if (UMessagingManager* Messaging = TargetLocalPlayer->GetSubsystem<UMessagingManager>())
		{
			const FDialogMessagingResultDelegate ResultCallback = FDialogMessagingResultDelegate::CreateUObject(this, &UAsyncAction_ShowDialogWidget::HandleConfirmationResult);
			Messaging->ShowDialogInternal(WidgetTag, Descriptor, ResultCallback);
			return;
		}
	}

	// 如果无法确认，就处理未知结果，什么都不播
	HandleConfirmationResult(FGameplayTag::EmptyTag);
}

void UAsyncAction_ShowDialogWidget::HandleConfirmationResult(FGameplayTag ConfirmationResult)
{
	OnResult.Broadcast(ConfirmationResult);

	SetReadyToDestroy();
}
