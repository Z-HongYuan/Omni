// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Gameplay/ExpGameMode.h"

#include "Components/ExpManagerComponent.h"
#include "Components/ExpPawnExtensionComponent.h"
#include "Components/ExpPlayerSpawningManagerComponent.h"
#include "Data/ExpDefinition.h"
#include "Data/ExpPawnData.h"
#include "Engine/AssetManager.h"
#include "GameFramework/GameStateBase.h"
#include "Gameplay/ExpAIController.h"
#include "Gameplay/ExpGameState.h"
#include "Gameplay/ExpPlayerState.h"
#include "Helper/ExpSystemSettings.h"
#include "Helper/ExpWorldSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Logs/LogExpSystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ExpGameMode)

AExpGameMode::AExpGameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 插件层只负责提供流程骨架，因此这里只设置体验系统强依赖的两个类
	// 新项目的 GameMode 子类需要自行设置以下类（参考 Lyra）：
	//   PlayerControllerClass / HUDClass / DefaultPawnClass / GameSessionClass / ReplaySpectatorPlayerControllerClass
	// 其中 DefaultPawnClass 只是「体验没配 PawnData 时的兜底」，正常由 UExpPawnData::PawnClass 决定
	GameStateClass = AExpGameState::StaticClass();
	PlayerStateClass = AExpPlayerState::StaticClass();
}

const UExpPawnData* AExpGameMode::GetPawnDataForController(const AController* InController) const
{
	// 优先从PS中获取PawnData,因为可能有自定义的PawnData,例如英雄选择系统
	if (InController != nullptr)
	{
		if (const AExpPlayerState* PS = InController->GetPlayerState<AExpPlayerState>())
		{
			if (const UExpPawnData* PawnData = PS->GetPawnData<UExpPawnData>())
			{
				return PawnData;
			}
		}
	}

	// 没有的话就回退到 体验中的默认PawnData
	check(GameState);
	UExpManagerComponent* ExperienceComponent = GameState->FindComponentByClass<UExpManagerComponent>();
	check(ExperienceComponent);

	if (ExperienceComponent->IsExperienceLoaded())
	{
		const UExpDefinition* Experience = ExperienceComponent->GetCurrentExperienceChecked();
		if (Experience->DefaultPawnData != nullptr)
		{
			return Experience->DefaultPawnData;
		}
	}

	// 体验还没加载完,并且PS中也没有设置
	return nullptr;
}

void AExpGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	// 下一帧
	GetWorldTimerManager().SetTimerForNextTick(this, &AExpGameMode::MatchExperience);
}

void AExpGameMode::InitGameState()
{
	Super::InitGameState();

	// 监听体验加载完成事件
	UExpManagerComponent* ExperienceComponent = GameState->FindComponentByClass<UExpManagerComponent>();
	check(ExperienceComponent);
	ExperienceComponent->CallOrRegister_OnExperienceLoaded(FExpLoaded::FDelegate::CreateUObject(this, &ThisClass::OnExperienceLoaded));
}

void AExpGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	// 全部玩家确保Experience加载完成后才开始生成玩家
	// 意味着全部拦截
	// 先加入的玩家,就会在 ExperienceLoaded 重新走一遍这里
	// 后加入的玩家,就会直接走这里然后生成玩家
	// 意味着没改原生逻辑,只是需要等待 ExperienceLoaded 加载完成,被拦截的就在回调中重新生成
	if (IsExperienceLoaded())
	{
		Super::HandleStartingNewPlayer_Implementation(NewPlayer);
	}
}

UClass* AExpGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	// 原生直接返回了 GameMode 中配置的 DefaultPawnClass
	// 这里改为使用体验资产中配置的 PawnClass
	if (const UExpPawnData* PawnData = GetPawnDataForController(InController))
		if (PawnData->PawnClass)
			return PawnData->PawnClass;

	// 如果不行的话还是回退到默认的 DefaultPawnClass
	return Super::GetDefaultPawnClassForController_Implementation(InController);
}

APawn* AExpGameMode::SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform)
{
	// 原生是生成 Pawn
	// 这里使用延迟生成,并且设置了其组件的 PawnData
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Instigator = GetInstigator();
	SpawnInfo.ObjectFlags |= RF_Transient; // 永远不要将默认玩家典当保存到地图中。
	SpawnInfo.bDeferConstruction = true; // 延迟构造,方便设置PawnData

	UClass* PawnClass = GetDefaultPawnClassForController(NewPlayer);
	if (!PawnClass)
	{
		UE_LOG(LogExpSystem, Error, TEXT("Game mode was unable to spawn Pawn due to NULL pawn class."));
		return nullptr;
	}

	APawn* SpawnedPawn = GetWorld()->SpawnActor<APawn>(PawnClass, SpawnTransform, SpawnInfo);
	if (!SpawnedPawn)
	{
		UE_LOG(LogExpSystem, Error, TEXT("Game mode was unable to spawn Pawn of class [%s] at [%s]."), *GetNameSafe(PawnClass), *SpawnTransform.ToHumanReadableString());
		return nullptr;
	}

	// 如果有插件的情况下才设置PawnData
	if (UExpPawnExtensionComponent* PawnExtComp = UExpPawnExtensionComponent::FindPawnExtensionComponent(SpawnedPawn))
	{
		if (const UExpPawnData* PawnData = GetPawnDataForController(NewPlayer))
		{
			PawnExtComp->SetPawnData(PawnData);
		}
		else
		{
			UE_LOG(LogExpSystem, Error, TEXT("Game mode was unable to set PawnData on the spawned pawn [%s]."), *GetNameSafe(SpawnedPawn));
		}
	}

	SpawnedPawn->FinishSpawning(SpawnTransform);

	return SpawnedPawn;
}

void AExpGameMode::GenericPlayerInitialization(AController* NewPlayer)
{
	Super::GenericPlayerInitialization(NewPlayer);

	// 广播玩家初始化完成,无论是玩家还是bot都会调用广播
	OnGameModePlayerInitialized.Broadcast(this, NewPlayer);
}

bool AExpGameMode::ShouldSpawnAtStartSpot(AController* Player)
{
	//永远不使用出生点，始终使用生成管理组件。 
	// ShouldSpawnAtStartSpot + UpdatePlayerStartSpot 就导致每次重生选点的时候必然会调用ChoosePlayerStart_Implementation,所以转发到生成组件内
	return false;
	// return Super::ShouldSpawnAtStartSpot(Player);
}

bool AExpGameMode::UpdatePlayerStartSpot(AController* Player, const FString& Portal, FString& OutErrorMessage)
{
	// return Super::UpdatePlayerStartSpot(Player, Portal, OutErrorMessage);
	// ShouldSpawnAtStartSpot + UpdatePlayerStartSpot 就导致每次重生选点的时候必然会调用ChoosePlayerStart_Implementation,所以转发到生成组件内
	// 原生逻辑是选点并且缓存地点
	// 详情看源码
	return true;
}

AActor* AExpGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	// 委托到 生成管理器里面接管,这样每个体验都可以自定义自己的选点规则
	if (UExpPlayerSpawningManagerComponent* PlayerSpawningComponent = GameState->FindComponentByClass<UExpPlayerSpawningManagerComponent>())
	{
		return PlayerSpawningComponent->ChoosePlayerStart(Player);
	}

	// 没有挂载生成管理组件时，退回引擎原生选点
	return Super::ChoosePlayerStart_Implementation(Player);
}

bool AExpGameMode::PlayerCanRestart_Implementation(APlayerController* Player)
{
	// 继承使用原生逻辑的情况下,同时支持 AIController 的重生判定
	return ControllerCanRestart(Player);
}

bool AExpGameMode::ControllerCanRestart(AController* Controller)
{
	if (APlayerController* PC = Cast<APlayerController>(Controller))
	{
		if (!Super::PlayerCanRestart_Implementation(PC))
		{
			return false;
		}
	}
	else // AIController
	{
		// 用于机器人的 Super::PlayerCanRestart_Implementation 重生判定
		if ((Controller == nullptr) || Controller->IsPendingKillPending())
		{
			return false;
		}
	}

	// 同时会在 生成管理器里面判断是否可以重生,项目层可在组件里接入复活冷却、回合制等规则
	if (UExpPlayerSpawningManagerComponent* PlayerSpawningComponent = GameState->FindComponentByClass<UExpPlayerSpawningManagerComponent>())
	{
		return PlayerSpawningComponent->ControllerCanRestart(Controller);
	}

	return true;
}

void AExpGameMode::FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation)
{
	// 委托到 生成管理器里面接管,管理器里面提供各种钩子用于处理重生时的逻辑
	if (UExpPlayerSpawningManagerComponent* PlayerSpawningComponent = GameState->FindComponentByClass<UExpPlayerSpawningManagerComponent>())
	{
		PlayerSpawningComponent->FinishRestartPlayer(NewPlayer, StartRotation);
	}

	Super::FinishRestartPlayer(NewPlayer, StartRotation);
}

void AExpGameMode::FailedToRestartPlayer(AController* NewPlayer)
{
	Super::FailedToRestartPlayer(NewPlayer);

	// ② 先确认这个控制器有可用的 Pawn 类（没类就没必要重试，重试也没用）
	UClass* PawnClass = GetDefaultPawnClassForController(NewPlayer);
	if (!PawnClass)
	{
		UE_LOG(LogExpSystem, Verbose, TEXT("...there's no pawn class so giving up."));
		return;
	}

	// ③ 玩家和 Bot 走不同的重试策略
	if (APlayerController* NewPC = Cast<APlayerController>(NewPlayer))
	{
		// 玩家：不无限循环，才允许重试时下一帧重试,前面重写的 PlayerCanRestart_Implementation 会判断是否可以重生 兼容 Bot
		if (PlayerCanRestart(NewPC))
		{
			// 玩家 → 下一帧尝试
			RequestPlayerRestartNextFrame(NewPlayer, false);
		}
		else
		{
			// 玩家已不能再重生 → 放弃，避免死循环
			UE_LOG(LogExpSystem, Verbose, TEXT("...PlayerCanRestart returned false, so we're not going to try again."));
		}
	}
	else
	{
		// Bot：无条件下一帧重试,机器人就一直重试
		RequestPlayerRestartNextFrame(NewPlayer, false);
	}
}

void AExpGameMode::RequestPlayerRestartNextFrame(AController* Controller, bool bForceReset)
{
	// 如果强制重置的话,直接重置控制器
	if (bForceReset && (Controller != nullptr))
	{
		Controller->Reset();
	}

	// 如果是玩家控制器,则调用玩家控制器的重生函数
	if (APlayerController* PC = Cast<APlayerController>(Controller))
	{
		GetWorldTimerManager().SetTimerForNextTick(PC, &APlayerController::ServerRestartPlayer_Implementation);
	}
	// 如果是 AI控制器,则调用 AI控制器的重生函数,内部会再次走一遍 ControllerCanRestart 判定
	if (AExpAIController* BotController = Cast<AExpAIController>(Controller))
	{
		GetWorldTimerManager().SetTimerForNextTick(BotController, &AExpAIController::ServerRestartController);
	}
}

void AExpGameMode::MatchExperience()
{
	FPrimaryAssetId ExperienceId;
	FString ExperienceIdSource;

	//优先顺序（最高获胜）
	//-配对任务（如有）
	//-URL选项覆盖
	//-开发人员设置（仅限PIE）
	//-命令行覆盖
	//-世界设置
	//-专用服务器
	//-默认体验

	UWorld* World = GetWorld();

	// 从Level流送的URL获取体验
	if (!ExperienceId.IsValid() && UGameplayStatics::HasOption(OptionsString, TEXT("Experience")))
	{
		const FString ExperienceFromOptions = UGameplayStatics::ParseOption(OptionsString, TEXT("Experience"));
		ExperienceId = FPrimaryAssetId(FPrimaryAssetType(UExpDefinition::StaticClass()->GetFName()), FName(*ExperienceFromOptions));
		ExperienceIdSource = TEXT("OptionsString");
	}

	// 从开发者设置中获取体验覆盖
	if (!ExperienceId.IsValid() && World->IsPlayInEditor())
	{
		ExperienceId = GetDefault<UExpSystemSettings>()->ExperienceOverride;
		ExperienceIdSource = TEXT("DeveloperSettings");
	}

	// 从命令行中获取需要的体验覆盖
	if (!ExperienceId.IsValid())
	{
		FString ExperienceFromCommandLine;
		if (FParse::Value(FCommandLine::Get(), TEXT("Experience="), ExperienceFromCommandLine))
		{
			ExperienceId = FPrimaryAssetId::ParseTypeAndName(ExperienceFromCommandLine);
			if (!ExperienceId.PrimaryAssetType.IsValid())
			{
				ExperienceId = FPrimaryAssetId(FPrimaryAssetType(UExpDefinition::StaticClass()->GetFName()), FName(*ExperienceFromCommandLine));
			}
			ExperienceIdSource = TEXT("CommandLine");
		}
	}

	// 从世界设置中获取体验
	if (!ExperienceId.IsValid())
	{
		if (AExpWorldSettings* TypedWorldSettings = Cast<AExpWorldSettings>(GetWorldSettings()))
		{
			ExperienceId = TypedWorldSettings->GetDefaultGameplayExperience();
			ExperienceIdSource = TEXT("WorldSettings");
		}
	}

	UAssetManager& AssetManager = UAssetManager::Get();
	FAssetData Dummy;
	if (ExperienceId.IsValid() && !AssetManager.GetPrimaryAssetData(ExperienceId, Dummy))
	{
		UE_LOG(LogExpSystem, Error, TEXT("EXPERIENCE: Wanted to use %s but couldn't find it, falling back to the default)"), *ExperienceId.ToString());
		ExperienceId = FPrimaryAssetId();
	}

	// 所有来源都识别不出来时：先给专用服务器一个接管的机会，再回退到项目层提供的兜底体验
	if (!ExperienceId.IsValid())
	{
		if (TryDedicatedServerLogin())
		{
			// 专用服务器会自己重新开一场并换图，这里不再往下走
			return;
		}

		ExperienceId = GetFallbackExperienceId();
		ExperienceIdSource = TEXT("Fallback");
	}

	// 最后开始整个体验
	MatchExperienceComplete(ExperienceId, ExperienceIdSource);
}

FPrimaryAssetId AExpGameMode::GetFallbackExperienceId() const
{
	// 框架层不预设任何资产名，由项目层 override 决定
	return FPrimaryAssetId();
}

bool AExpGameMode::TryDedicatedServerLogin()
{
	// 默认不接管专用服务器登录，由项目层 override 实现
	return false;
}

void AExpGameMode::HostDedicatedServerMatch()
{
	// 具体的专用服务器建局策略由项目层 override 实现
}

void AExpGameMode::MatchExperienceComplete(const FPrimaryAssetId& ExperienceId, const FString& ExperienceIdSource)
{
	// 拿到之后就设置开始在组件上启动体验的加载流程
	if (ExperienceId.IsValid())
	{
		UE_LOG(LogExpSystem, Log, TEXT("Identified experience %s (Source: %s)"), *ExperienceId.ToString(), *ExperienceIdSource);

		UExpManagerComponent* ExperienceComponent = GameState->FindComponentByClass<UExpManagerComponent>();
		check(ExperienceComponent);
		ExperienceComponent->SetCurrentExperience(ExperienceId);
	}
	else
	{
		UE_LOG(LogExpSystem, Error, TEXT("Failed to identify experience, loading screen will stay up forever. ")
		       TEXT("请在世界设置里配置 DefaultGameplayExperience，或在项目层重载 AExpGameMode::GetFallbackExperienceId() 提供兜底体验。"));
	}
}

void AExpGameMode::OnExperienceLoaded(const UExpDefinition* ExperienceDefinition)
{
	//生成任何已连接的玩家
	//@TODO:这里我们只处理*player*控制器，但在GetDefaultPawnClassForController_Implementation中，我们跳过了所有控制器
	//GetDefaultPawnClassForController_Implement可能无论如何只会被玩家调用
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PC = Cast<APlayerController>(*Iterator);
		if ((PC != nullptr) && (PC->GetPawn() == nullptr))
		{
			if (PlayerCanRestart(PC))
			{
				RestartPlayer(PC);
			}
		}
	}
}

bool AExpGameMode::IsExperienceLoaded() const
{
	check(GameState);
	UExpManagerComponent* ExperienceComponent = GameState->FindComponentByClass<UExpManagerComponent>();
	check(ExperienceComponent);

	return ExperienceComponent->IsExperienceLoaded();
}
