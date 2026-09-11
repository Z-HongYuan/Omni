// Copyright © 2026 张鸿源. All Rights Reserved.

#include "Actions/AsyncAction_CreateWidget.h"

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Engine/AssetManager.h"
#include "Engine/Engine.h"
#include "Engine/LocalPlayer.h"
#include "Engine/StreamableManager.h"
#include "HelperFunctions/UIHelperFunctions.h"
#include "LogAdvancedUI.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AsyncAction_CreateWidget)

static const FName InputFilterReason_Template = FName(TEXT("CreatingWidgetAsync"));

UAsyncAction_CreateWidget* UAsyncAction_CreateWidget::CreateWidgetAsync(UObject* WorldContextObject, TSoftClassPtr<UUserWidget> InUserWidgetSoftClass, APlayerController* InOwningPlayer, bool bSuspendInputUntilComplete)
{
	if (InUserWidgetSoftClass.IsNull())
	{
		UE_LOG(LogAdvancedUI, Warning, TEXT("CreateWidgetAsync: 未指定控件类。"));
		return nullptr;
	}

	UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	if (!World || !World->GetGameInstance())
	{
		UE_LOG(LogAdvancedUI, Warning, TEXT("CreateWidgetAsync: 无法获取有效的世界或游戏实例。"));
		return nullptr;
	}

	UAsyncAction_CreateWidget* Action = NewObject<UAsyncAction_CreateWidget>();
	Action->UserWidgetSoftClass = InUserWidgetSoftClass;
	Action->OwningPlayer = InOwningPlayer;
	Action->World = World;
	Action->GameInstance = World->GetGameInstance();
	Action->bSuspendInputUntilComplete = bSuspendInputUntilComplete;
	Action->RegisterWithGameInstance(World);
	return Action;
}

void UAsyncAction_CreateWidget::Activate()
{
	if (bActivated || bFinished) return;
	bActivated = true;

	if (!World.IsValid() || !GameInstance.IsValid() || OwningPlayer.IsStale())
	{
		Complete(nullptr);
		return;
	}

	InputLocalPlayer = OwningPlayer.IsValid() ? OwningPlayer->GetLocalPlayer() : nullptr;
	SuspendInputToken = bSuspendInputUntilComplete ? UUIHelperFunctions::SuspendInputForPlayer(InputLocalPlayer.Get(), InputFilterReason_Template) : NAME_None;

	// 先绑定取消回调再启动，确保已缓存的资源也遵守相同的收尾流程。
	StreamingHandle = UAssetManager::Get().GetStreamableManager().RequestAsyncLoad(
		UserWidgetSoftClass.ToSoftObjectPath(),
		FStreamableDelegate::CreateUObject(this, &ThisClass::OnWidgetLoaded),
		FStreamableManager::AsyncLoadHighPriority, false, true);

	if (!StreamingHandle)
	{
		Complete(nullptr);
		return;
	}

	StreamingHandle->BindCancelDelegate(FStreamableDelegate::CreateUObject(this, &ThisClass::Cancel));
	StreamingHandle->StartStalledHandle();
}

void UAsyncAction_CreateWidget::ResumeInput()
{
	// 输入令牌属于本地玩家；控制器更换或销毁不影响令牌的归还。
	UUIHelperFunctions::ResumeInputForPlayer(InputLocalPlayer.Get(), SuspendInputToken);
	SuspendInputToken = NAME_None;
}

void UAsyncAction_CreateWidget::Cancel()
{
	if (bFinished) return;
	bFinished = true;
	ResumeInput();

	TSharedPtr<FStreamableHandle> Handle = MoveTemp(StreamingHandle);
	Super::Cancel();
	if (Handle) Handle->CancelHandle();
}

void UAsyncAction_CreateWidget::Complete(UUserWidget* Widget)
{
	if (bFinished) return;
	bFinished = true;
	ResumeInput();
	StreamingHandle.Reset();

	if (ShouldBroadcastDelegates()) OnComplete.Broadcast(Widget);
	SetReadyToDestroy();
}

void UAsyncAction_CreateWidget::OnWidgetLoaded()
{
	if (bFinished) return;

	UUserWidget* Widget = nullptr;
	UClass* WidgetClass = UserWidgetSoftClass.Get();
	if (World.IsValid() && GameInstance.IsValid() && !OwningPlayer.IsStale()
		&& WidgetClass && WidgetClass->IsChildOf<UUserWidget>() && !WidgetClass->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists))
	{
		Widget = UWidgetBlueprintLibrary::Create(World.Get(), WidgetClass, OwningPlayer.Get());
	}
	Complete(Widget);
}
