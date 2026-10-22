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

/** 在 GameInstance 生命周期内汇总加载请求，并管理加载画面、输入拦截和临时性能设置。 */
UCLASS(MinimalAPI)
class ULoadingScreenManager : public UGameInstanceSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

public:
	//~USubsystem 接口
	UE_API virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	UE_API virtual void Deinitialize() override;
	UE_API virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	//~USubsystem 接口结束

	//~FTickableObjectBase 接口
	UE_API virtual void Tick(float DeltaTime) override;
	UE_API virtual ETickableTickType GetTickableTickType() const override;
	UE_API virtual bool IsTickable() const override;
	UE_API virtual TStatId GetStatId() const override;
	UE_API virtual UWorld* GetTickableGameObjectWorld() const override;
	//~FTickableObjectBase 接口结束

	/** 返回最近一次显示或隐藏加载画面的诊断原因。 */
	UFUNCTION(BlueprintCallable, Category="LoadingScreen")
	FString GetDebugReasonForShowingOrHidingLoadingScreen() const { return DebugReasonForShowingOrHidingLoadingScreen; }

	/** 返回当前是否正在显示加载画面。 */
	UFUNCTION(BlueprintCallable, Category="LoadingScreen")
	bool GetLoadingScreenDisplayStatus() const { return bCurrentlyShowingLoadingScreen; }

	/** 加载画面可见性变化时广播。 */
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnLoadingScreenVisibilityChangedDelegate, bool);
	FORCEINLINE FOnLoadingScreenVisibilityChangedDelegate& OnLoadingScreenVisibilityChangedDelegate() { return LoadingScreenVisibilityChanged; }

	UE_API void RegisterLoadingProcessor(const TScriptInterface<ILoadingScreenCheckInterface>& Interface);
	UE_API void UnregisterLoadingProcessor(const TScriptInterface<ILoadingScreenCheckInterface>& Interface);

protected:
	void HandlePreLoadMap(const FWorldContext& WorldContext, const FString& MapName);
	void HandlePostLoadMap(UWorld* World);

	/** 确定是否应该显示或隐藏加载屏幕。每帧调用。 */
	void UpdateLoadingScreen();

	/** 查询地图、网络连接及外部处理器是否仍要求显示加载画面。 */
	bool CheckForAnyNeedToShowLoadingScreen();

	/** 结合加载请求、调试开关和额外停留时间，决定是否显示加载画面。 */
	bool ShouldShowLoadingScreen();

	/** 返回引擎预加载管理器是否仍有活动界面。 */
	bool IsShowingInitialLoadingScreen() const;

	/** 创建并显示加载控件，同时启用输入拦截和加载性能设置。 */
	void ShowLoadingScreen();

	/** 移除加载控件，并恢复输入和临时性能设置。 */
	void HideLoadingScreen();

	/** 从游戏视口和各本地玩家视口移除已创建的加载控件。 */
	void RemoveWidgetFromViewport();

	/** 注册输入预处理器；编辑器中的拦截由调试开关决定。 */
	void StartBlockingInput();

	/** 注销输入预处理器，并释放其共享引用。 */
	void StopBlockingInput();

	/** 成对启用或恢复加载期间的性能与心跳设置。 */
	void ChangePerformanceSettings(bool bEnableLoadingScreen);

private:
	/** 加载画面可见性变化通知。 */
	FOnLoadingScreenVisibilityChangedDelegate LoadingScreenVisibilityChanged;

	/** 非分屏路径持有的加载控件；未创建或已清理时为空。 */
	TSharedPtr<SWidget> LoadingScreenWidget;
	TMap<TWeakObjectPtr<ULocalPlayer>, TSharedPtr<SWidget>> PlayersLoadingScreenWidgets;

	/** 加载期间注册的 Slate 输入预处理器。 */
	TSharedPtr<IInputProcessor> InputPreProcessor;

	/** 外部加载处理器的弱引用；注册不会延长处理器生命周期。 */
	TArray<TWeakInterfacePtr<ILoadingScreenCheckInterface>> ExternalLoadingProcessors;

	/** 最近一次显示或隐藏加载画面的诊断原因。 */
	FString DebugReasonForShowingOrHidingLoadingScreen;

	/** 开始显示加载画面时的平台时间。 */
	double TimeLoadingScreenShown = 0.0;

	/** 加载条件最近解除时的平台时间，用于计算额外停留时长。 */
	double TimeLoadingScreenLastDismissed = -1.0;

	/** 距离下一次加载原因日志输出的剩余秒数。 */
	double TimeUntilNextLogHeartbeatSeconds = 0.0;

	/** 是否处于 PreLoadMap 与 PostLoadMap 通知之间。 */
	bool bCurrentlyInLoadMap = false;

	/** 是否已进入加载画面显示状态。 */
	bool bCurrentlyShowingLoadingScreen = false;

	/** 是否已启用加载性能设置，用于保证正常隐藏和退出清理只恢复一次。 */
	bool bLoadingPerformanceSettingsApplied = false;
};

#undef UE_API
