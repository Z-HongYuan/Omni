// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Actions/AsyncAction_PushWidgetToLayerForPlayer.h"

#include "Engine/Engine.h"
#include "Engine/StreamableManager.h"
#include "LogAdvancedUI.h"
#include "Widgets/GameRootLayoutWidget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AsyncAction_PushWidgetToLayerForPlayer)

UAsyncAction_PushWidgetToLayerForPlayer* UAsyncAction_PushWidgetToLayerForPlayer::PushWidgetToLayerForPlayer(APlayerController* InOwningPlayer, TSoftClassPtr<UCommonActivatableWidget> InWidgetClass, FGameplayTag InLayerName,
                                                                                                             bool bSuspendInputUntilComplete)
{
	if (InWidgetClass.IsNull())
	{
		FFrame::KismetExecutionMessage(TEXT("PushContentToLayerForPlayer was passed a null WidgetClass"), ELogVerbosity::Error);
		return nullptr;
	}

	if (UWorld* World = GEngine->GetWorldFromContextObject(InOwningPlayer, EGetWorldErrorMode::LogAndReturnNull))
	{
		UAsyncAction_PushWidgetToLayerForPlayer* Action = NewObject<UAsyncAction_PushWidgetToLayerForPlayer>();
		Action->WidgetClass = InWidgetClass;
		Action->OwningPlayerPtr = InOwningPlayer;
		Action->LayerName = InLayerName;
		Action->bSuspendInputUntilComplete = bSuspendInputUntilComplete;
		Action->RegisterWithGameInstance(World);

		return Action;
	}

	return nullptr;
}

void UAsyncAction_PushWidgetToLayerForPlayer::Activate()
{
	if (UGameRootLayoutWidget* RootLayout = UGameRootLayoutWidget::GetRootLayoutWidget(OwningPlayerPtr.Get()))
	{
		TWeakObjectPtr<UAsyncAction_PushWidgetToLayerForPlayer> WeakThis(this);
		StreamingHandle = RootLayout->PushWidgetToLayerStackAsync<UCommonActivatableWidget>(
			LayerName, bSuspendInputUntilComplete, WidgetClass, [this, WeakThis](EAsyncWidgetPushState State, UCommonActivatableWidget* Widget)
			{
				if (WeakThis.IsValid())
				{
					switch (State)
					{
					case EAsyncWidgetPushState::Initialize:
						BeforePush.Broadcast(Widget);
						break;
					case EAsyncWidgetPushState::AfterPush:
						AfterPush.Broadcast(Widget);
						SetReadyToDestroy();
						break;
					case EAsyncWidgetPushState::Canceled:
						SetReadyToDestroy();
						break;
					}
				}
				SetReadyToDestroy();
			});
	}
	else
	{
		// 无法获取根控件(例如 UI 策略未初始化),记录日志避免调用方无感知
		UE_LOG(LogAdvancedUI, Warning, TEXT("UAsyncAction_PushWidgetToLayerForPlayer::Activate: Failed to get root layout widget for layer [%s], action canceled"), *LayerName.ToString());
		SetReadyToDestroy();
	}
}

void UAsyncAction_PushWidgetToLayerForPlayer::Cancel()
{
	Super::Cancel();

	if (StreamingHandle.IsValid())
	{
		StreamingHandle->CancelHandle();
		StreamingHandle.Reset();
	}
}
