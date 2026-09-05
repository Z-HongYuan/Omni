// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Tickable.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UObject/WeakInterfacePtr.h"
#include "LoadingScreenManager.generated.h"

#define UE_API LOADINGSCREEN_API

class ULocalPlayer;
class SWidget;
class IInputProcessor;
class ILoadingScreenCheckInterface;

/**
 * 整个游戏生命周期的加载屏幕管理器
 */
UCLASS(MinimalAPI)
class ULoadingScreenManager : public UGameInstanceSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

public:
	//~USubsystem 接口
	UE_API virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	UE_API virtual void Deinitialize() override;
	UE_API virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	//~End USubsystem interface

	//~FTickableObjectBase 接口
	UE_API virtual void Tick(float DeltaTime) override;
	UE_API virtual ETickableTickType GetTickableTickType() const override;
	UE_API virtual bool IsTickable() const override;
	UE_API virtual TStatId GetStatId() const override;
	UE_API virtual UWorld* GetTickableGameObjectWorld() const override;
	//~End FTickableObjectBase interface

	// 获取显示加载屏幕的原因(Debug)
	UFUNCTION(BlueprintCallable, Category="LoadingScreen")
	FString GetDebugReasonForShowingOrHidingLoadingScreen() const { return DebugReasonForShowingOrHidingLoadingScreen; }

	// 返回当前是否正在显示加载屏幕
	UFUNCTION(BlueprintCallable, Category="LoadingScreen")
	bool GetLoadingScreenDisplayStatus() const { return bCurrentlyShowingLoadingScreen; }

	// 当加载屏幕可见性变化时调用
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnLoadingScreenVisibilityChangedDelegate, bool);
	FORCEINLINE FOnLoadingScreenVisibilityChangedDelegate& OnLoadingScreenVisibilityChangedDelegate() { return LoadingScreenVisibilityChanged; }

	UE_API void RegisterLoadingProcessor(const TScriptInterface<ILoadingScreenCheckInterface>& Interface);
	UE_API void UnregisterLoadingProcessor(const TScriptInterface<ILoadingScreenCheckInterface>& Interface);

protected:
	void HandlePreLoadMap(const FWorldContext& WorldContext, const FString& MapName);
	void HandlePostLoadMap(UWorld* World);

	/** 确定是否应该显示或隐藏加载屏幕。每帧调用。 */
	void UpdateLoadingScreen();

	/** 如果需要显示加载屏幕则返回 true。覆盖大多数默认检查时机 */
	bool CheckForAnyNeedToShowLoadingScreen();

	/** 如果我们想要显示加载屏幕（如果需要或因其他原因强制开启）则返回 true。包括手动显示 */
	bool ShouldShowLoadingScreen();

	/** 如果处于不应显示此屏幕的初始加载流程中则返回 true */
	bool IsShowingInitialLoadingScreen() const;

	/** 显示加载屏幕。在视口上显示加载屏幕控件 */
	void ShowLoadingScreen();

	/** 隐藏加载屏幕。加载屏幕控件将被销毁 */
	void HideLoadingScreen();

	/** 从视口中移除控件 */
	void RemoveWidgetFromViewport();

	/** 当加载屏幕可见时阻止游戏中的输入 */
	void StartBlockingInput();

	/** 恢复游戏输入（如果被阻止） */
	void StopBlockingInput();

	/** 改变性能设置,例如画面设置之类 */
	void ChangePerformanceSettings(bool bEnableLoadingScreen);

private:
	/** 当加载屏幕可见性变化时广播的委托 */
	FOnLoadingScreenVisibilityChangedDelegate LoadingScreenVisibilityChanged;

	/** 对正在显示的加载屏幕控件的引用 可能为空 */
	TSharedPtr<SWidget> LoadingScreenWidget;
	TMap<TWeakObjectPtr<ULocalPlayer>, TSharedPtr<SWidget>> PlayersLoadingScreenWidgets;

	/** 在显示加载屏幕的时候,阻止所有的玩家输入 */
	TSharedPtr<IInputProcessor> InputPreProcessor;

	/** 外部加载处理器，可能是需要加载屏幕显示的 Actor 或者 Task */
	TArray<TWeakInterfacePtr<ILoadingScreenCheckInterface>> ExternalLoadingProcessors;

	/** 加载屏幕显示（或不显示）的原因 */
	FString DebugReasonForShowingOrHidingLoadingScreen;

	/** 开始显示加载屏幕的时间 */
	double TimeLoadingScreenShown = 0.0;

	/** 加载屏幕最近希望被关闭的时间（可能由于最小显示时长要求仍然显示） */
	double TimeLoadingScreenLastDismissed = -1.0;

	/** 下次记录加载屏幕仍显示原因的日志时间 */
	double TimeUntilNextLogHeartbeatSeconds = 0.0;

	/** 当处于 PreLoadMap 和 PostLoadMap 之间时为 true */
	bool bCurrentlyInLoadMap = false;

	/** 当加载屏幕当前正在显示时为 true */
	bool bCurrentlyShowingLoadingScreen = false;
};

#undef UE_API
