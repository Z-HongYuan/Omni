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
	// 插件设置必需的 GameState 和 PlayerState，其余默认类型由项目配置。
	// PawnData.PawnClass 优先于 GameMode 的 DefaultPawnClass。
	GameStateClass = AExpGameState::StaticClass();
	PlayerStateClass = AExpPlayerState::StaticClass();
}

const UExpPawnData* AExpGameMode::GetPawnDataForController(const AController* InController) const
{
	// 优先使用 PlayerState 的 PawnData，保留角色选择等项目配置。
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

	// PlayerState 没有配置时，尝试已就绪体验的默认 PawnData。
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

	// 尚无可用 PawnData，交由调用方处理兜底。
	return nullptr;
}

void AExpGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	// 延后一帧选择体验，使 GameState 等游戏对象先完成创建。
	GetWorldTimerManager().SetTimerForNextTick(this, &AExpGameMode::MatchExperience);
}

void AExpGameMode::InitGameState()
{
	Super::InitGameState();

	// 体验就绪时处理此前等待生成的玩家。
	UExpManagerComponent* ExperienceComponent = GameState->FindComponentByClass<UExpManagerComponent>();
	check(ExperienceComponent);
	ExperienceComponent->CallOrRegister_OnExperienceLoaded(FExpLoaded::FDelegate::CreateUObject(this, &ThisClass::OnExperienceLoaded));
}

void AExpGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	// 体验就绪前暂缓父类生成流程；已连接且没有 Pawn 的玩家由 OnExperienceLoaded 请求重生。
	if (IsExperienceLoaded())
	{
		Super::HandleStartingNewPlayer_Implementation(NewPlayer);
	}
}

UClass* AExpGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	// 优先使用控制器对应 PawnData 的角色类型。
	if (const UExpPawnData* PawnData = GetPawnDataForController(InController))
		if (PawnData->PawnClass)
			return PawnData->PawnClass;

	// 未配置 PawnClass 时回退到 GameMode 默认类型。
	return Super::GetDefaultPawnClassForController_Implementation(InController);
}

APawn* AExpGameMode::SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform)
{
	// 延迟构造 Pawn，使 PawnData 能在完成生成前注入扩展组件。
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Instigator = GetInstigator();
	SpawnInfo.ObjectFlags |= RF_Transient; // 运行时玩家 Pawn 不保存到地图。
	SpawnInfo.bDeferConstruction = true; // 完成构造前先注入 PawnData。

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

	// 仅对包含 Pawn 扩展组件的角色注入 PawnData。
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

	// 玩家和 Bot 都通过此入口广播初始化完成。
	OnGameModePlayerInitialized.Broadcast(this, NewPlayer);
}

bool AExpGameMode::ShouldSpawnAtStartSpot(AController* Player)
{
	// 每次重生重新选点，不复用控制器缓存的 StartSpot。
	return false;
	// return Super::ShouldSpawnAtStartSpot(Player);
}

bool AExpGameMode::UpdatePlayerStartSpot(AController* Player, const FString& Portal, FString& OutErrorMessage)
{
	// return Super::UpdatePlayerStartSpot(Player, Portal, OutErrorMessage);
	// 此处不预选和缓存出生点，交由后续 ChoosePlayerStart 流程选择。
	return true;
}

AActor* AExpGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	// 优先转发给生成管理组件，允许体验定制选点。
	if (UExpPlayerSpawningManagerComponent* PlayerSpawningComponent = GameState->FindComponentByClass<UExpPlayerSpawningManagerComponent>())
	{
		return PlayerSpawningComponent->ChoosePlayerStart(Player);
	}

	// 未挂载生成管理组件时使用引擎选点。
	return Super::ChoosePlayerStart_Implementation(Player);
}

bool AExpGameMode::PlayerCanRestart_Implementation(APlayerController* Player)
{
	// 将玩家和 AI 纳入统一重生检查入口。
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
		// 非玩家控制器只检查存在性和待销毁状态，不调用 PlayerController 专用检查。
		if ((Controller == nullptr) || Controller->IsPendingKillPending())
		{
			return false;
		}
	}

	// 存在生成管理组件时，再由其检查是否允许重生。
	if (UExpPlayerSpawningManagerComponent* PlayerSpawningComponent = GameState->FindComponentByClass<UExpPlayerSpawningManagerComponent>())
	{
		return PlayerSpawningComponent->ControllerCanRestart(Controller);
	}

	return true;
}

void AExpGameMode::FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation)
{
	// 先通知生成管理组件的重生钩子，再执行父类收尾。
	if (UExpPlayerSpawningManagerComponent* PlayerSpawningComponent = GameState->FindComponentByClass<UExpPlayerSpawningManagerComponent>())
	{
		PlayerSpawningComponent->FinishRestartPlayer(NewPlayer, StartRotation);
	}

	Super::FinishRestartPlayer(NewPlayer, StartRotation);
}

void AExpGameMode::FailedToRestartPlayer(AController* NewPlayer)
{
	Super::FailedToRestartPlayer(NewPlayer);

	// 没有可用 Pawn 类型时停止重试，避免无效循环。
	UClass* PawnClass = GetDefaultPawnClassForController(NewPlayer);
	if (!PawnClass)
	{
		UE_LOG(LogExpSystem, Verbose, TEXT("...there's no pawn class so giving up."));
		return;
	}

	// 玩家与 Bot 使用各自的重试入口。
	if (APlayerController* NewPC = Cast<APlayerController>(NewPlayer))
	{
		// 玩家仍被允许重生时才安排下一帧重试。
		if (PlayerCanRestart(NewPC))
		{
			// 下一帧重试，避免同帧递归生成。
			RequestPlayerRestartNextFrame(NewPlayer, false);
		}
		else
		{
			// 不再允许重生时停止重试。
			UE_LOG(LogExpSystem, Verbose, TEXT("...PlayerCanRestart returned false, so we're not going to try again."));
		}
	}
	else
	{
		// Bot 安排下一帧请求，控制器入口仍会检查状态和重生权限。
		RequestPlayerRestartNextFrame(NewPlayer, false);
	}
}

void AExpGameMode::RequestPlayerRestartNextFrame(AController* Controller, bool bForceReset)
{
	// 强制重置在当前帧执行，重生请求延后。
	if (bForceReset && (Controller != nullptr))
	{
		Controller->Reset();
	}

	// 玩家控制器使用其服务端重生入口。
	if (APlayerController* PC = Cast<APlayerController>(Controller))
	{
		GetWorldTimerManager().SetTimerForNextTick(PC, &APlayerController::ServerRestartPlayer_Implementation);
	}
	// Bot 使用专用重生入口，内部再次检查 ControllerCanRestart。
	if (AExpAIController* BotController = Cast<AExpAIController>(Controller))
	{
		GetWorldTimerManager().SetTimerForNextTick(BotController, &AExpAIController::ServerRestartController);
	}
}

void AExpGameMode::MatchExperience()
{
	FPrimaryAssetId ExperienceId;
	FString ExperienceIdSource;

	// 依次尝试 URL、PIE 开发者设置、命令行和世界设置。
	// 选中的主资产校验失败时直接转入服务器接管或项目兜底，不再尝试较低优先级来源。

	UWorld* World = GetWorld();

	// 优先读取旅行 URL 的 Experience 选项。
	if (!ExperienceId.IsValid() && UGameplayStatics::HasOption(OptionsString, TEXT("Experience")))
	{
		const FString ExperienceFromOptions = UGameplayStatics::ParseOption(OptionsString, TEXT("Experience"));
		ExperienceId = FPrimaryAssetId(FPrimaryAssetType(UExpDefinition::StaticClass()->GetFName()), FName(*ExperienceFromOptions));
		ExperienceIdSource = TEXT("OptionsString");
	}

	// 仅 PIE 可使用开发者体验覆盖。
	if (!ExperienceId.IsValid() && World->IsPlayInEditor())
	{
		ExperienceId = GetDefault<UExpSystemSettings>()->ExperienceOverride;
		ExperienceIdSource = TEXT("DeveloperSettings");
	}

	// 再尝试命令行 Experience 参数。
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

	// 仍未选择体验时读取世界设置。
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

	// 体验无效时先尝试专用服务器接管，再读取项目兜底体验。
	if (!ExperienceId.IsValid())
	{
		if (TryDedicatedServerLogin())
		{
			// 派生实现已接管后续流程，此处停止选择体验。
			return;
		}

		ExperienceId = GetFallbackExperienceId();
		ExperienceIdSource = TEXT("Fallback");
	}

	// 将选择结果交给体验管理器。
	MatchExperienceComplete(ExperienceId, ExperienceIdSource);
}

FPrimaryAssetId AExpGameMode::GetFallbackExperienceId() const
{
	// 默认不指定资产，项目子类可提供兜底体验。
	return FPrimaryAssetId();
}

bool AExpGameMode::TryDedicatedServerLogin()
{
	// 默认不接管专用服务器登录。
	return false;
}

void AExpGameMode::HostDedicatedServerMatch()
{
	// 建局策略由项目子类实现。
}

void AExpGameMode::MatchExperienceComplete(const FPrimaryAssetId& ExperienceId, const FString& ExperienceIdSource)
{
	// 有效体验由 GameState 上的管理组件加载。
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
	// 为已连接且尚无 Pawn 的玩家请求重生。
	// TODO: 确认体验就绪前已存在的非玩家控制器是否需要对应的恢复生成流程。
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
