// Copyright © 2026 张鸿源. All Rights Reserved.


#include "LoadingScreenManager.h"

#include "LoadingScreenCheckInterface.h"
#include "LoadingScreenDeveloperSettings.h"
#include "PreLoadScreenManager.h"
#include "ShaderPipelineCache.h"
#include "Blueprint/UserWidget.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/GameViewportClient.h"
#include "Framework/Application/IInputProcessor.h"
#include "Framework/Application/SlateApplication.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/WorldSettings.h"
#include "HAL/ThreadHeartBeat.h"
#include "Misc/ConfigCacheIni.h"
#include "Widgets/Images/SThrobber.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LoadingScreenCheckInterface)
#include UE_INLINE_GENERATED_CPP_BY_NAME(LoadingScreenDeveloperSettings)
#include UE_INLINE_GENERATED_CPP_BY_NAME(LoadingScreenManager)

DECLARE_LOG_CATEGORY_EXTERN(LogLoadingScreen, Log, All);
DEFINE_LOG_CATEGORY(LogLoadingScreen);

CSV_DEFINE_CATEGORY(LoadingScreen, true);

// 加载画面的控制台配置。
namespace LoadingScreenCVars
{
	static float HoldLoadingScreenAdditionalSecs = 2.0f;
	static FAutoConsoleVariableRef CVarHoldLoadingScreenUpAtLeastThisLongInSecs(
		TEXT("LoadingScreen.HoldLoadingScreenAdditionalSecs"),
		HoldLoadingScreenAdditionalSecs,
		TEXT("其他加载结束后，额外保持加载画面的秒数，用于等待纹理流送、减少画面模糊。"),
		ECVF_Default | ECVF_Preview);

	static bool LogLoadingScreenReasonEveryFrame = false;
	static FAutoConsoleVariableRef CVarLogLoadingScreenReasonEveryFrame(
		TEXT("LoadingScreen.LogLoadingScreenReasonEveryFrame"),
		LogLoadingScreenReasonEveryFrame,
		TEXT("如果为真，加载画面显示或隐藏的原因会在每帧都显示在日志中。"),
		ECVF_Default);

	static bool ForceLoadingScreenVisible = false;
	static FAutoConsoleVariableRef CVarForceLoadingScreenVisible(
		TEXT("LoadingScreen.AlwaysShow"),
		ForceLoadingScreenVisible,
		TEXT("强制显示加载界面。"),
		ECVF_Default);

	static bool LoadingScreenAlwaysStopPlayerInput = false;
	static FAutoConsoleVariableRef CVarLoadingScreenAlwaysStopPlayerInput(
		TEXT("LoadingScreen.AlwaysStopPlayerInput"),
		LoadingScreenAlwaysStopPlayerInput,
		TEXT("加载画面显示时，在编辑器中也拦截 Slate 输入，用于验证输入阻塞。"),
		ECVF_Default);
}

/** 在加载画面显示期间拦截 Slate 输入；编辑器中由调试开关决定是否拦截。 */
class FLoadingScreenInputPreProcessor : public IInputProcessor
{
public:
	FLoadingScreenInputPreProcessor() = default;
	virtual ~FLoadingScreenInputPreProcessor() override = default;

	static bool CanStopInput() { return !GIsEditor || LoadingScreenCVars::LoadingScreenAlwaysStopPlayerInput; }

	//~IInputProcess 接口
	virtual void Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor) override
	{
	}

	virtual bool HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override { return CanStopInput(); }
	virtual bool HandleKeyUpEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override { return CanStopInput(); }
	virtual bool HandleAnalogInputEvent(FSlateApplication& SlateApp, const FAnalogInputEvent& InAnalogInputEvent) override { return CanStopInput(); }
	virtual bool HandleMouseMoveEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override { return CanStopInput(); }
	virtual bool HandleMouseButtonDownEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override { return CanStopInput(); }
	virtual bool HandleMouseButtonUpEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override { return CanStopInput(); }
	virtual bool HandleMouseButtonDoubleClickEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override { return CanStopInput(); }
	virtual bool HandleMouseWheelOrGestureEvent(FSlateApplication& SlateApp, const FPointerEvent& InWheelEvent, const FPointerEvent* InGestureEvent) override { return CanStopInput(); }
	virtual bool HandleMotionDetectedEvent(FSlateApplication& SlateApp, const FMotionEvent& MotionEvent) override { return CanStopInput(); }
	//~IInputProcess 接口结束
};


// 子系统接口
void ULoadingScreenManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	FCoreUObjectDelegates::PreLoadMapWithContext.AddUObject(this, &ThisClass::HandlePreLoadMap);
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &ThisClass::HandlePostLoadMap);

	const UGameInstance* LocalGameInstance = GetGameInstance();
	check(LocalGameInstance);
}

void ULoadingScreenManager::Deinitialize()
{
	// 先停止更新并解除回调，防止清理过程中再次启动加载画面。
	SetTickableTickType(ETickableTickType::Never);
	FCoreUObjectDelegates::PreLoadMapWithContext.RemoveAll(this);
	FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);

	StopBlockingInput();
	RemoveWidgetFromViewport();
	ChangePerformanceSettings(false);
	FThreadHeartBeat::Get().MonitorCheckpointEnd(GetFName());

	bCurrentlyShowingLoadingScreen = false;
	bCurrentlyInLoadMap = false;
	ExternalLoadingProcessors.Reset();
	LoadingScreenVisibilityChanged.Clear();

	Super::Deinitialize();
}

bool ULoadingScreenManager::ShouldCreateSubsystem(UObject* Outer) const
{
	// 专用服务器不创建加载画面子系统。
	const UGameInstance* GameInstance = CastChecked<UGameInstance>(Outer);
	const bool bIsServerWorld = GameInstance->IsDedicatedServerInstance();
	return !bIsServerWorld;
}

// 逐帧更新接口
void ULoadingScreenManager::Tick(float DeltaTime)
{
	UpdateLoadingScreen();

	TimeUntilNextLogHeartbeatSeconds = FMath::Max(TimeUntilNextLogHeartbeatSeconds - DeltaTime, 0.0);
}

ETickableTickType ULoadingScreenManager::GetTickableTickType() const
{
	if (IsTemplate())
	{
		return ETickableTickType::Never;
	}
	return ETickableTickType::Conditional;
}

bool ULoadingScreenManager::IsTickable() const
{
	// 只有游戏视口存在时才更新加载画面。
	UGameInstance* GameInstance = GetGameInstance();
	return (GameInstance && GameInstance->GetGameViewportClient());
}

TStatId ULoadingScreenManager::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(ULoadingScreenManager, STATGROUP_Tickables);
}

UWorld* ULoadingScreenManager::GetTickableGameObjectWorld() const
{
	return GetGameInstance()->GetWorld();
}

// 外部加载请求
void ULoadingScreenManager::RegisterLoadingProcessor(const TScriptInterface<ILoadingScreenCheckInterface>& Interface)
{
	ExternalLoadingProcessors.Add(Interface.GetObject());
}

void ULoadingScreenManager::UnregisterLoadingProcessor(const TScriptInterface<ILoadingScreenCheckInterface>& Interface)
{
	ExternalLoadingProcessors.Remove(Interface.GetObject());
}

// 加载流程与界面控制
void ULoadingScreenManager::HandlePreLoadMap(const FWorldContext& WorldContext, const FString& MapName)
{
	if (WorldContext.OwningGameInstance == GetGameInstance())
	{
		bCurrentlyInLoadMap = true;

		// 引擎已初始化时立即更新，避免地图加载前漏过首次显示。
		if (GEngine->IsInitialized())
		{
			UpdateLoadingScreen();
		}
	}
}

void ULoadingScreenManager::HandlePostLoadMap(UWorld* World)
{
	if ((World != nullptr) && (World->GetGameInstance() == GetGameInstance()))
	{
		bCurrentlyInLoadMap = false;
	}
}

void ULoadingScreenManager::UpdateLoadingScreen()
{
	bool bLogLoadingScreenStatus = LoadingScreenCVars::LogLoadingScreenReasonEveryFrame;

	if (ShouldShowLoadingScreen())
	{
		const ULoadingScreenDeveloperSettings* Settings = GetDefault<ULoadingScreenDeveloperSettings>();

		// 如果我们未能在规定时间内到达指定的检查点，会触发卡顿检测器，以便更好地判断进度停滞的位置。
		FThreadHeartBeat::Get().MonitorCheckpointStart(GetFName(), Settings->LoadingScreenHeartbeatHangDuration);

		ShowLoadingScreen();

		if ((Settings->LogLoadingScreenHeartbeatInterval > 0.0f) && (TimeUntilNextLogHeartbeatSeconds <= 0.0))
		{
			bLogLoadingScreenStatus = true;
			TimeUntilNextLogHeartbeatSeconds = Settings->LogLoadingScreenHeartbeatInterval;
		}
	}
	else
	{
		HideLoadingScreen();

		FThreadHeartBeat::Get().MonitorCheckpointEnd(GetFName());
	}

	if (bLogLoadingScreenStatus)
	{
		UE_LOG(LogLoadingScreen, Log, TEXT("Loading screen showing: %d. Reason: %s"), bCurrentlyShowingLoadingScreen ? 1 : 0, *DebugReasonForShowingOrHidingLoadingScreen);
	}
}

bool ULoadingScreenManager::CheckForAnyNeedToShowLoadingScreen()
{
	// 设置兜底原因，便于发现未填写诊断信息的返回分支。
	DebugReasonForShowingOrHidingLoadingScreen = TEXT("Reason for Showing/Hiding LoadingScreen is unknown!");

	const UGameInstance* LocalGameInstance = GetGameInstance();

	// 检查开发者设置
	if (LoadingScreenCVars::ForceLoadingScreenVisible)
	{
		DebugReasonForShowingOrHidingLoadingScreen = FString(TEXT("LoadingScreen.AlwaysShow is true"));
		return true;
	}

	// 检查世界上下文和世界
	const FWorldContext* Context = LocalGameInstance->GetWorldContext();
	if (Context == nullptr)
	{
		DebugReasonForShowingOrHidingLoadingScreen = FString(TEXT("The game instance has a null WorldContext"));
		return true;
	}

	// 检查世界是否有效
	UWorld* World = Context->World();
	if (World == nullptr)
	{
		DebugReasonForShowingOrHidingLoadingScreen = FString(TEXT("We have no world (FWorldContext's World() is null)"));
		return true;
	}

	// GameState 尚未创建或复制到本地时继续等待。
	AGameStateBase* GameState = World->GetGameState<AGameStateBase>();
	if (GameState == nullptr)
	{
		DebugReasonForShowingOrHidingLoadingScreen = FString(TEXT("GameState hasn't yet replicated (it's null)"));
		return true;
	}

	// 检查是否在加载地图
	if (bCurrentlyInLoadMap)
	{
		DebugReasonForShowingOrHidingLoadingScreen = FString(TEXT("bCurrentlyInLoadMap is true"));
		return true;
	}

	// 存在待处理的地图跳转时继续显示。
	if (!Context->TravelURL.IsEmpty())
	{
		DebugReasonForShowingOrHidingLoadingScreen = FString(TEXT("We have pending travel (the TravelURL is not empty)"));
		return true;
	}

	// 检查是否正在连接其他服务器
	if (Context->PendingNetGame != nullptr)
	{
		DebugReasonForShowingOrHidingLoadingScreen = FString(TEXT("We are connecting to another server (PendingNetGame != nullptr)"));
		return true;
	}

	// 检查世界是否已开始
	if (!World->HasBegunPlay())
	{
		DebugReasonForShowingOrHidingLoadingScreen = FString(TEXT("World hasn't begun play"));
		return true;
	}

	// 无缝切图尚未结束时继续显示。
	if (World->IsInSeamlessTravel())
	{
		DebugReasonForShowingOrHidingLoadingScreen = FString(TEXT("We are in seamless travel"));
		return true;
	}

	// 检查游戏状态是否需要显示
	if (ILoadingScreenCheckInterface::ShouldShowLoadingScreen(GameState, DebugReasonForShowingOrHidingLoadingScreen))
	{
		return true;
	}

	// 检查游戏状态的组件是否需要显示
	for (UActorComponent* TestComponent : GameState->GetComponents())
	{
		if (ILoadingScreenCheckInterface::ShouldShowLoadingScreen(TestComponent, DebugReasonForShowingOrHidingLoadingScreen))
		{
			return true;
		}
	}

	// 查询外部处理器是否仍有加载请求。
	for (const TWeakInterfacePtr<ILoadingScreenCheckInterface>& Processor : ExternalLoadingProcessors)
	{
		if (ILoadingScreenCheckInterface::ShouldShowLoadingScreen(Processor.GetObject(), DebugReasonForShowingOrHidingLoadingScreen))
		{
			return true;
		}
	}

	// 本地玩家及其控制器也可提出加载请求。
	bool bFoundAnyLocalPC = false;
	bool bMissingAnyLocalPC = false;

	for (ULocalPlayer* LP : LocalGameInstance->GetLocalPlayers())
	{
		if (LP != nullptr)
		{
			if (APlayerController* PC = LP->PlayerController)
			{
				bFoundAnyLocalPC = true;

				// 查询玩家控制器的加载请求。
				if (ILoadingScreenCheckInterface::ShouldShowLoadingScreen(PC, DebugReasonForShowingOrHidingLoadingScreen))
				{
					return true;
				}

				// 查询玩家控制器组件的加载请求。
				for (UActorComponent* TestComponent : PC->GetComponents())
				{
					if (ILoadingScreenCheckInterface::ShouldShowLoadingScreen(TestComponent, DebugReasonForShowingOrHidingLoadingScreen))
					{
						return true;
					}
				}
			}
			else
			{
				bMissingAnyLocalPC = true;
			}
		}
	}

	UGameViewportClient* GameViewportClient = LocalGameInstance->GetGameViewportClient();
	const bool bIsInSplitscreen = GameViewportClient->GetCurrentSplitscreenConfiguration() != ESplitScreenType::None;

	// 分屏模式要求每个本地玩家都有控制器。
	if (bIsInSplitscreen && bMissingAnyLocalPC)
	{
		DebugReasonForShowingOrHidingLoadingScreen = FString(TEXT("At least one missing local player controller in splitscreen"));
		return true;
	}

	// 非分屏模式至少需要一个本地玩家控制器。
	if (!bIsInSplitscreen && !bFoundAnyLocalPC)
	{
		DebugReasonForShowingOrHidingLoadingScreen = FString(TEXT("Need at least one local player controller"));
		return true;
	}

	DebugReasonForShowingOrHidingLoadingScreen = TEXT("(nothing wants to show it anymore)");
	return false;
}

bool ULoadingScreenManager::ShouldShowLoadingScreen()
{
	const ULoadingScreenDeveloperSettings* Settings = GetDefault<ULoadingScreenDeveloperSettings>();

	// 非 Shipping 构建允许通过命令行关闭加载画面。
#if !UE_BUILD_SHIPPING
	static bool bCmdLineNoLoadingScreen = FParse::Param(FCommandLine::Get(), TEXT("NoLoadingScreen"));
	if (bCmdLineNoLoadingScreen)
	{
		DebugReasonForShowingOrHidingLoadingScreen = FString(TEXT("CommandLine has 'NoLoadingScreen'"));
		return false;
	}
#endif

	UGameInstance* LocalGameInstance = GetGameInstance();
	if (LocalGameInstance->GetGameViewportClient() == nullptr)
	{
		return false;
	}

	const bool bNeedToShowLoadingScreen = CheckForAnyNeedToShowLoadingScreen();

	// 真实加载结束后，可继续保留画面一段配置时长。
	bool bWantToForceShowLoadingScreen = false;
	if (bNeedToShowLoadingScreen)
	{
		TimeLoadingScreenLastDismissed = -1.0;
	}
	else
	{
		// 从加载条件首次解除时开始计算额外停留时间。
		const double CurrentTime = FPlatformTime::Seconds();
		const bool bCanHoldLoadingScreen = (!GIsEditor || Settings->HoldLoadingScreenAdditionalSecsEvenInEditor);
		const double HoldLoadingScreenAdditionalSecs = bCanHoldLoadingScreen ? LoadingScreenCVars::HoldLoadingScreenAdditionalSecs : 0.0;

		if (TimeLoadingScreenLastDismissed < 0.0)
		{
			TimeLoadingScreenLastDismissed = CurrentTime;
		}
		const double TimeSinceScreenDismissed = CurrentTime - TimeLoadingScreenLastDismissed;

		// 额外停留只为纹理流送预留时间，不代表流送已完成。
		if ((HoldLoadingScreenAdditionalSecs > 0.0) && (TimeSinceScreenDismissed < HoldLoadingScreenAdditionalSecs))
		{
			// 此时恢复世界渲染，让可见资源开始流送。
			// 待完善：如果额外等待期间再次需要加载画面，目前不会重新关闭世界渲染。
			UGameViewportClient* GameViewportClient = GetGameInstance()->GetGameViewportClient();
			GameViewportClient->bDisableWorldRendering = false;

			DebugReasonForShowingOrHidingLoadingScreen = FString::Printf(TEXT("Keeping loading screen up for an additional %.2f seconds to allow texture streaming"), HoldLoadingScreenAdditionalSecs);
			bWantToForceShowLoadingScreen = true;
		}
	}

	return bNeedToShowLoadingScreen || bWantToForceShowLoadingScreen;
}

bool ULoadingScreenManager::IsShowingInitialLoadingScreen() const
{
	const FPreLoadScreenManager* PreLoadScreenManager = FPreLoadScreenManager::Get();
	return (PreLoadScreenManager != nullptr) && PreLoadScreenManager->HasValidActivePreLoadScreen();
}

void ULoadingScreenManager::ShowLoadingScreen()
{
	if (bCurrentlyShowingLoadingScreen) return;

	// 引擎启动画面仍在显示时，暂不接管加载画面。
	if (FPreLoadScreenManager::Get() && FPreLoadScreenManager::Get()->HasActivePreLoadScreenType(EPreLoadScreenTypes::EngineLoadingScreen)) return;

	TimeLoadingScreenShown = FPlatformTime::Seconds();

	bCurrentlyShowingLoadingScreen = true;

	CSV_EVENT(LoadingScreen, TEXT("Show"));

	const ULoadingScreenDeveloperSettings* Settings = GetDefault<ULoadingScreenDeveloperSettings>();

	if (IsShowingInitialLoadingScreen())
	{
		UE_LOG(LogLoadingScreen, Log, TEXT("Showing loading screen when 'IsShowingInitialLoadingScreen()' is true."));
		UE_LOG(LogLoadingScreen, Log, TEXT("%s"), *DebugReasonForShowingOrHidingLoadingScreen);
	}
	else
	{
		UE_LOG(LogLoadingScreen, Log, TEXT("Showing loading screen when 'IsShowingInitialLoadingScreen()' is false."));
		UE_LOG(LogLoadingScreen, Log, TEXT("%s"), *DebugReasonForShowingOrHidingLoadingScreen);

		UGameInstance* LocalGameInstance = GetGameInstance();

		StartBlockingInput();

		LoadingScreenVisibilityChanged.Broadcast(true);

		TSubclassOf<UUserWidget> LoadingScreenWidgetClass = Settings->LoadingScreenWidget.TryLoadClass<UUserWidget>();
		UGameViewportClient* GameViewportClient = LocalGameInstance->GetGameViewportClient();

		if (GameViewportClient->bEnablePlayersSplitRT)
		{
			for (ULocalPlayer* Player : LocalGameInstance->GetLocalPlayers())
			{
				if (Player)
				{
					TSharedPtr<SWidget> PlayerWidget;

					if (UUserWidget* UserWidget = UUserWidget::CreateWidgetInstance(*LocalGameInstance, LoadingScreenWidgetClass, NAME_None))
					{
						PlayerWidget = UserWidget->TakeWidget();
					}
					else
					{
						UE_LOG(LogLoadingScreen, Error, TEXT("Failed to load the loading screen widget %s, falling back to placeholder."), *Settings->LoadingScreenWidget.ToString());
						PlayerWidget = SNew(SThrobber);
					}

					PlayersLoadingScreenWidgets.Add(Player, PlayerWidget);
					GameViewportClient->AddViewportWidgetForPlayer(Player, PlayerWidget.ToSharedRef(), Settings->LoadingScreenZOrder);
				}
			}
		}
		else
		{
			if (UUserWidget* UserWidget = UUserWidget::CreateWidgetInstance(*LocalGameInstance, LoadingScreenWidgetClass, NAME_None))
			{
				LoadingScreenWidget = UserWidget->TakeWidget();
			}
			else
			{
				UE_LOG(LogLoadingScreen, Error, TEXT("Failed to load the loading screen widget %s, falling back to placeholder."), *Settings->LoadingScreenWidget.ToString());
				LoadingScreenWidget = SNew(SThrobber);
			}

			// 使用配置的高层级覆盖游戏视口内容。
			GameViewportClient->AddViewportWidgetContent(LoadingScreenWidget.ToSharedRef(), Settings->LoadingScreenZOrder);
		}

		ChangePerformanceSettings(true);

		if (!GIsEditor || Settings->ForceTickLoadingScreenEvenInEditor)
		{
			// 立即更新 Slate，避免加载画面等到下一帧才出现。
			FSlateApplication::Get().Tick();
		}
	}
}

void ULoadingScreenManager::HideLoadingScreen()
{
	if (!bCurrentlyShowingLoadingScreen) return;

	StopBlockingInput();

	if (IsShowingInitialLoadingScreen())
	{
		UE_LOG(LogLoadingScreen, Log, TEXT("Hiding loading screen when 'IsShowingInitialLoadingScreen()' is true."));
		UE_LOG(LogLoadingScreen, Log, TEXT("%s"), *DebugReasonForShowingOrHidingLoadingScreen);
	}
	else
	{
		UE_LOG(LogLoadingScreen, Log, TEXT("Hiding loading screen when 'IsShowingInitialLoadingScreen()' is false."));
		UE_LOG(LogLoadingScreen, Log, TEXT("%s"), *DebugReasonForShowingOrHidingLoadingScreen);

		UE_LOG(LogLoadingScreen, Log, TEXT("Garbage Collecting before dropping load screen"));
		GEngine->ForceGarbageCollection(true);

		RemoveWidgetFromViewport();

		ChangePerformanceSettings(false);

		LoadingScreenVisibilityChanged.Broadcast(false);
	}

	CSV_EVENT(LoadingScreen, TEXT("Hide"));

	const double LoadingScreenDuration = FPlatformTime::Seconds() - TimeLoadingScreenShown;
	UE_LOG(LogLoadingScreen, Log, TEXT("LoadingScreen was visible for %.2fs"), LoadingScreenDuration);

	bCurrentlyShowingLoadingScreen = false;
}

void ULoadingScreenManager::RemoveWidgetFromViewport()
{
	const UGameInstance* LocalGameInstance = GetGameInstance();
	UGameViewportClient* GameViewportClient = LocalGameInstance ? LocalGameInstance->GetGameViewportClient() : nullptr;

	// 清理已创建的控件，不依赖当前分屏模式；退出时视口也可能已经失效。
	if (GameViewportClient != nullptr)
	{
		for (const TPair<TWeakObjectPtr<ULocalPlayer>, TSharedPtr<SWidget>>& Pair : PlayersLoadingScreenWidgets)
		{
			if (Pair.Key.IsValid() && Pair.Value.IsValid())
			{
				GameViewportClient->RemoveViewportWidgetForPlayer(Pair.Key.Get(), Pair.Value.ToSharedRef());
			}
		}
		if (LoadingScreenWidget.IsValid())
		{
			GameViewportClient->RemoveViewportWidgetContent(LoadingScreenWidget.ToSharedRef());
		}
	}
	PlayersLoadingScreenWidgets.Reset();
	LoadingScreenWidget.Reset();
}

void ULoadingScreenManager::StartBlockingInput()
{
	if (!InputPreProcessor.IsValid())
	{
		InputPreProcessor = MakeShareable<FLoadingScreenInputPreProcessor>(new FLoadingScreenInputPreProcessor());
		FSlateApplication::Get().RegisterInputPreProcessor(InputPreProcessor, 0);
	}
}

void ULoadingScreenManager::StopBlockingInput()
{
	if (InputPreProcessor.IsValid())
	{
		if (FSlateApplication::IsInitialized())
		{
			FSlateApplication::Get().UnregisterInputPreProcessor(InputPreProcessor);
		}
		InputPreProcessor.Reset();
	}
}

void ULoadingScreenManager::ChangePerformanceSettings(bool bEnableLoadingScreen)
{
	// 只撤销本实例实际启用的设置，避免重复退出或从未显示过界面时多次恢复心跳。
	if (bLoadingPerformanceSettingsApplied == bEnableLoadingScreen)
	{
		return;
	}
	bLoadingPerformanceSettingsApplied = bEnableLoadingScreen;

	const UGameInstance* LocalGameInstance = GetGameInstance();
	UGameViewportClient* GameViewportClient = LocalGameInstance ? LocalGameInstance->GetGameViewportClient() : nullptr;

	FShaderPipelineCache::SetBatchMode(bEnableLoadingScreen ? FShaderPipelineCache::BatchMode::Fast : FShaderPipelineCache::BatchMode::Background);

	// 视口失效时仍须恢复下方的全局性能与心跳状态。
	if (GameViewportClient)
	{
		// 加载时停止绘制世界，并提高关卡流送的优先级。
		GameViewportClient->bDisableWorldRendering = bEnableLoadingScreen;
		if (UWorld* ViewportWorld = GameViewportClient->GetWorld())
		{
			if (AWorldSettings* WorldSettings = ViewportWorld->GetWorldSettings(false, false))
			{
				WorldSettings->bHighPriorityLoadingLocal = bEnableLoadingScreen;
			}
		}
	}

	if (bEnableLoadingScreen)
	{
		// 当加载画面可见时，设置新的卡顿检测超时乘数。
		double HangDurationMultiplier;
		if (!GConfig || !GConfig->GetDouble(TEXT("Core.System"), TEXT("LoadingScreenHangDurationMultiplier"), HangDurationMultiplier, GEngineIni))
		{
			HangDurationMultiplier = 1.0;
		}
		FThreadHeartBeat::Get().SetDurationMultiplier(HangDurationMultiplier);

		// 加载画面可见期间暂停游戏线程卡顿上报。
		FGameThreadHitchHeartBeat::Get().SuspendHeartBeat();
	}
	else
	{
		// 隐藏画面时恢复默认挂起检测超时。
		FThreadHeartBeat::Get().SetDurationMultiplier(1.0);

		// 恢复游戏线程卡顿上报。
		FGameThreadHitchHeartBeat::Get().ResumeHeartBeat();
	}
}
