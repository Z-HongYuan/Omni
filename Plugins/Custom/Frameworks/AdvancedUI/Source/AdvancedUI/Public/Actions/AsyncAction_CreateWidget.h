// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Engine/CancellableAsyncAction.h"
#include "AsyncAction_CreateWidget.generated.h"

#define UE_API ADVANCEDUI_API

class UUserWidget;
struct FStreamableHandle;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCreateWidgetAsyncDelegate, UUserWidget*, UserWidget);

/**
 * 异步加载控件类，加载完成后创建控件实例，并在 OnComplete 返回指针。
 */
UCLASS(MinimalAPI, BlueprintType)
class UAsyncAction_CreateWidget : public UCancellableAsyncAction
{
	GENERATED_BODY()

public:
	/*
	 * 异步加载控件类,并且在加载完成后创建控件实例并返回
	 * 可选择是否暂停输入
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, meta=(WorldContext = "WorldContextObject", BlueprintInternalUseOnly="true"), Category=AdvancedUI)
	static UE_API UAsyncAction_CreateWidget* CreateWidgetAsync(UObject* WorldContextObject, TSoftClassPtr<UUserWidget> InUserWidgetSoftClass, APlayerController* InOwningPlayer, bool bSuspendInputUntilComplete = true);

	UE_API virtual void Activate() override;
	UE_API virtual void Cancel() override;

	UPROPERTY(BlueprintAssignable)
	FCreateWidgetAsyncDelegate OnComplete;

private:
	void OnWidgetLoaded();

	FName SuspendInputToken;
	TWeakObjectPtr<APlayerController> OwningPlayer;
	TWeakObjectPtr<UWorld> World;
	TWeakObjectPtr<UGameInstance> GameInstance;
	bool bSuspendInputUntilComplete = true;
	TSoftClassPtr<UUserWidget> UserWidgetSoftClass;
	TSharedPtr<FStreamableHandle> StreamingHandle;
};

#undef UE_API
