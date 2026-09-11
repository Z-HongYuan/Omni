// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Engine/CancellableAsyncAction.h"
#include "AsyncAction_CreateWidget.generated.h"

#define UE_API GAMEUI_API

class UUserWidget;
class ULocalPlayer;
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
	 * 可选择是否暂停输入；加载或创建失败时返回空控件，主动取消时不广播。
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, meta=(WorldContext = "WorldContextObject", BlueprintInternalUseOnly="true"), Category=GameUI)
	static UE_API UAsyncAction_CreateWidget* CreateWidgetAsync(UObject* WorldContextObject, TSoftClassPtr<UUserWidget> InUserWidgetSoftClass, APlayerController* InOwningPlayer, bool bSuspendInputUntilComplete = true);

	UE_API virtual void Activate() override;
	UE_API virtual void Cancel() override;

	UPROPERTY(BlueprintAssignable)
	FCreateWidgetAsyncDelegate OnComplete;

private:
	void OnWidgetLoaded();
	void Complete(UUserWidget* Widget);
	void ResumeInput();

	FName SuspendInputToken;
	TWeakObjectPtr<ULocalPlayer> InputLocalPlayer;
	TWeakObjectPtr<APlayerController> OwningPlayer;
	TWeakObjectPtr<UWorld> World;
	TWeakObjectPtr<UGameInstance> GameInstance;
	bool bSuspendInputUntilComplete = true;
	bool bActivated = false;
	bool bFinished = false;
	TSoftClassPtr<UUserWidget> UserWidgetSoftClass;
	TSharedPtr<FStreamableHandle> StreamingHandle;
};

#undef UE_API
