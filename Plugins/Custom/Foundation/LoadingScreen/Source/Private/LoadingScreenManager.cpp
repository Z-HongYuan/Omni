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

DECLARE_LOG_CATEGORY_EXTERN(LogLoadingScreen, Log, All); // 注册日志分类
DEFINE_LOG_CATEGORY(LogLoadingScreen);

// 加载屏幕的分析类别
CSV_DEFINE_CATEGORY(LoadingScreen, true);

// 控制台变量
//
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

// FLoadingScreenInputPreProcessor 输入处理器
// 当显示加载屏幕时,插入输入处理器,用于捕获全部输入,以实现输入阻塞
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
//
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
	// 只能在客户端上显示
	const UGameInstance* GameInstance = CastChecked<UGameInstance>(Outer);
	const bool bIsServerWorld = GameInstance->IsDedicatedServerInstance();
	return !bIsServerWorld;
}

// 逐帧更新接口
//
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
	// 游戏客户端视口存在,才能显示加载画面,如果有其他问题也能发现
	UGameInstance* GameInstance = GetGameInstance();
	return (GameInstance && GameInstance->GetGameViewportClient());
}

TStatId ULoadingScreenManager::GetStatId() const
{
	// 性能统计
	RETURN_QUICK_DECLARE_CYCLE_STAT(ULoadingScreenManager, STATGROUP_Tickables);
}

UWorld* ULoadingScreenManager::GetTickableGameObjectWorld() const
{
	return GetGameInstance()->GetWorld();
}

// 外部加载请求
//
void ULoadingScreenManager::RegisterLoadingProcessor(const TScriptInterface<ILoadingScreenCheckInterface>& Interface)
{
	ExternalLoadingProcessors.Add(Interface.GetObject());
}

void ULoadingScreenManager::UnregisterLoadingProcessor(const TScriptInterface<ILoadingScreenCheckInterface>& Interface)
{
	ExternalLoadingProcessors.Remove(Interface.GetObject());
}

// 加载流程与界面控制
//
void ULoadingScreenManager::HandlePreLoadMap(const FWorldContext& WorldContext, const FString& MapName)
{
	if (WorldContext.OwningGameInstance == GetGameInstance())
	{
		bCurrentlyInLoadMap = true;

		// 如果引擎被初始化，加载界面会立即更新
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
	// 先写“未知”原因，以防以后有人忘记写原因。
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

	// 检查游戏状态是否准备好 (是否被复制)
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

	// 检查是否正在转换网络地图
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

	// 检查是否正在进行 seamless travel (无缝转换地图)
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

	// 检查外部加载屏幕处理器是否需要
	for (const TWeakInterfacePtr<ILoadingScreenCheckInterface>& Processor : ExternalLoadingProcessors)
	{
		if (ILoadingScreenCheckInterface::ShouldShowLoadingScreen(Processor.GetObject(), DebugReasonForShowingOrHidingLoadingScreen))
		{
			return true;
		}
	}

	// 检查每个本地玩家
	bool bFoundAnyLocalPC = false;
	bool bMissingAnyLocalPC = false;

	for (ULocalPlayer* LP : LocalGameInstance->GetLocalPlayers())
	{
		if (LP != nullptr)
		{
			if (APlayerController* PC = LP->PlayerController)
			{
				bFoundAnyLocalPC = true;

				// 查询每个本地玩家的 PC 是否需要
				if (ILoadingScreenCheckInterface::ShouldShowLoadingScreen(PC, DebugReasonForShowingOrHidingLoadingScreen))
				{
					return true;
				}

				// 查询每个本地玩家的 PC的组件是否需要
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

	// 查询是否全部本地玩家的 PC 都存在 (分屏模式下)
	if (bIsInSplitscreen && bMissingAnyLocalPC)
	{
		DebugReasonForShowingOrHidingLoadingScreen = FString(TEXT("At least one missing local player controller in splitscreen"));
		return true;
	}

	// 在非分屏模式下，需要至少一个本地玩家控制器存在
	if (!bIsInSplitscreen && !bFoundAnyLocalPC)
	{
		DebugReasonForShowingOrHidingLoadingScreen = FString(TEXT("Need at least one local player controller"));
		return true;
	}

	// 到这就不需要显示加载屏幕了
	DebugReasonForShowingOrHidingLoadingScreen = TEXT("(nothing wants to show it anymore)");
	return false;
}

bool ULoadingScreenManager::ShouldShowLoadingScreen()
{
	const ULoadingScreenDeveloperSettings* Settings = GetDefault<ULoadingScreenDeveloperSettings>();

	// 检查那些强制状态的调试命令
#if !UE_BUILD_SHIPPING
	static bool bCmdLineNoLoadingScreen = FParse::Param(FCommandLine::Get(), TEXT("NoLoadingScreen"));
	if (bCmdLineNoLoadingScreen)
	{
		DebugReasonForShowingOrHidingLoadingScreen = FString(TEXT("CommandLine has 'NoLoadingScreen'"));
		return false;
	}
#endif

	// 检查游戏视口是否存在
	UGameInstance* LocalGameInstance = GetGameInstance();
	if (LocalGameInstance->GetGameViewportClient() == nullptr)
	{
		return false;
	}

	// 检查是否需要显示加载画面 (默认检测的时机)
	const bool bNeedToShowLoadingScreen = CheckForAnyNeedToShowLoadingScreen();

	// 如果需要，可以让加载画面稍微延长一点
	bool bWantToForceShowLoadingScreen = false;
	if (bNeedToShowLoadingScreen)
	{
		// 还需要显示加载画面
		TimeLoadingScreenLastDismissed = -1.0;
	}
	else
	{
		// 不需要再显示屏幕了，但可能还想展示一阵子
		const double CurrentTime = FPlatformTime::Seconds();
		const bool bCanHoldLoadingScreen = (!GIsEditor || Settings->HoldLoadingScreenAdditionalSecsEvenInEditor);
		const double HoldLoadingScreenAdditionalSecs = bCanHoldLoadingScreen ? LoadingScreenCVars::HoldLoadingScreenAdditionalSecs : 0.0;

		if (TimeLoadingScreenLastDismissed < 0.0)
		{
			TimeLoadingScreenLastDismissed = CurrentTime;
		}
		const double TimeSinceScreenDismissed = CurrentTime - TimeLoadingScreenLastDismissed;

		// 多持续几秒,保证贴图流送或者其他的加载事件
		if ((HoldLoadingScreenAdditionalSecs > 0.0) && (TimeSinceScreenDismissed < HoldLoadingScreenAdditionalSecs))
		{
			// 确保我们此时渲染世界，这样贴图才能真正流入
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

	// 如果引擎还在加载界面，无法显示加载画面。
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

		// 阻挡玩家输入
		StartBlockingInput();

		LoadingScreenVisibilityChanged.Broadcast(true);

		// 创建加载界面小部件
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

			// 把视口设置在高ZOrder上，确保它能覆盖大多数物体
			GameViewportClient->AddViewportWidgetContent(LoadingScreenWidget.ToSharedRef(), Settings->LoadingScreenZOrder);
		}

		ChangePerformanceSettings(true);

		if (!GIsEditor || Settings->ForceTickLoadingScreenEvenInEditor)
		{
			// 启用 Tick Slate确保加载画面立即显示
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

		// 让观察者知道加载画面已经结束,广播委托
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

		// 加载界面出现时不要报告卡顿
		FGameThreadHitchHeartBeat::Get().SuspendHeartBeat();
	}
	else
	{
		// 当我们隐藏加载界面时，恢复卡顿检测超时
		FThreadHeartBeat::Get().SetDurationMultiplier(1.0);

		// 加载画面关闭后，恢复报告卡顿了
		FGameThreadHitchHeartBeat::Get().ResumeHeartBeat();
	}
}
