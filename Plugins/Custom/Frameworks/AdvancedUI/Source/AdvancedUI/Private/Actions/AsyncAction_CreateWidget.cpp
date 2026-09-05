// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Actions/AsyncAction_CreateWidget.h"

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Engine/AssetManager.h"
#include "Engine/Engine.h"
#include "HelperFunctions/UIHelperFunctions.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AsyncAction_CreateWidget)

static const FName InputFilterReason_Template = FName(TEXT("CreatingWidgetAsync"));

UAsyncAction_CreateWidget* UAsyncAction_CreateWidget::CreateWidgetAsync(UObject* WorldContextObject, TSoftClassPtr<UUserWidget> InUserWidgetSoftClass, APlayerController* InOwningPlayer, bool bSuspendInputUntilComplete)
{
	if (InUserWidgetSoftClass.IsNull())
	{
		FFrame::KismetExecutionMessage(TEXT("CreateWidgetAsync was passed a null UserWidgetSoftClass"), ELogVerbosity::Error);
		return nullptr;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);

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
	SuspendInputToken = bSuspendInputUntilComplete ? UUIHelperFunctions::SuspendInputForPlayer(OwningPlayer.Get(), InputFilterReason_Template) : NAME_None;

	StreamingHandle = UAssetManager::Get().GetStreamableManager().RequestAsyncLoad(
		UserWidgetSoftClass.ToSoftObjectPath(),
		FStreamableDelegate::CreateUObject(this, &ThisClass::OnWidgetLoaded),
		FStreamableManager::AsyncLoadHighPriority
	);

	// 绑定一个取消事件,用于恢复输入
	StreamingHandle->BindCancelDelegate(FStreamableDelegate::CreateWeakLambda(
			this, [this]()
			{
				UUIHelperFunctions::ResumeInputForPlayer(OwningPlayer.Get(), SuspendInputToken);
			})
	);
}

void UAsyncAction_CreateWidget::Cancel()
{
	Super::Cancel();

	if (StreamingHandle.IsValid())
	{
		StreamingHandle->CancelHandle();
		StreamingHandle.Reset();
	}
}

void UAsyncAction_CreateWidget::OnWidgetLoaded()
{
	if (bSuspendInputUntilComplete)
	{
		UUIHelperFunctions::ResumeInputForPlayer(OwningPlayer.Get(), SuspendInputToken);
	}

	// 如果加载完成后有效,就返回创建好的实例指针,否则不要调用完成
	if (TSubclassOf<UUserWidget> UserWidgetClass = UserWidgetSoftClass.Get())
	{
		UUserWidget* UserWidget = UWidgetBlueprintLibrary::Create(World.Get(), UserWidgetClass, OwningPlayer.Get());
		OnComplete.Broadcast(UserWidget);
	}

	StreamingHandle.Reset();

	SetReadyToDestroy();
}
