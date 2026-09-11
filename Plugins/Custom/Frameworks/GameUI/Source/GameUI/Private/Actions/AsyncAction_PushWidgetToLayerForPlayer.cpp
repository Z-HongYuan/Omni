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
		FFrame::KismetExecutionMessage(TEXT("PushWidgetToLayerForPlayer: 未指定控件类。"), ELogVerbosity::Error);
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
	if (bActivated || bFinished) return;
	bActivated = true;

	UGameRootLayoutWidget* RootLayout = UGameRootLayoutWidget::GetRootLayoutWidget(OwningPlayerPtr.Get());
	if (!RootLayout)
	{
		UE_LOG(LogAdvancedUI, Warning, TEXT("PushWidgetToLayerForPlayer: 无法获取层 [%s] 的根布局，推送已取消。"), *LayerName.ToString());
		Cancel();
		return;
	}

	const TWeakObjectPtr<UAsyncAction_PushWidgetToLayerForPlayer> WeakThis(this);
	TSharedPtr<FStreamableHandle> Handle = RootLayout->PushWidgetToLayerStackAsync<UCommonActivatableWidget>(
		LayerName, bSuspendInputUntilComplete, WidgetClass, [WeakThis](EAsyncWidgetPushState State, UCommonActivatableWidget* Widget)
		{
			UAsyncAction_PushWidgetToLayerForPlayer* Action = WeakThis.Get();
			if (!Action || Action->bFinished) return;

			if (State == EAsyncWidgetPushState::Initialize)
			{
				if (Action->ShouldBroadcastDelegates()) Action->BeforePush.Broadcast(Widget);
				return;
			}

			// 只有成功、失败或取消才结束动作，初始化回调期间保持注册。
			Action->bFinished = true;
			if (State == EAsyncWidgetPushState::AfterPush && Action->ShouldBroadcastDelegates())
			{
				Action->AfterPush.Broadcast(Widget);
			}
			Action->StreamingHandle.Reset();
			Action->SetReadyToDestroy();
		});

	if (!bFinished) StreamingHandle = MoveTemp(Handle);
}

void UAsyncAction_PushWidgetToLayerForPlayer::Cancel()
{
	if (bFinished) return;
	bFinished = true;
	TSharedPtr<FStreamableHandle> Handle = MoveTemp(StreamingHandle);
	Super::Cancel();
	if (Handle) Handle->CancelHandle();
}
