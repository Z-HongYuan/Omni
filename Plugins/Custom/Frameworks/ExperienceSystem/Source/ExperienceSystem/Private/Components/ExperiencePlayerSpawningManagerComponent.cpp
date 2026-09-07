// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Components/ExperiencePlayerSpawningManagerComponent.h"

#include "Engine/PlayerStartPIE.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerState.h"
#include "Gameplay/ExperiencePlayerStart.h"
#include "Logs/LogExperienceSystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ExperiencePlayerSpawningManagerComponent)

UExperiencePlayerSpawningManagerComponent::UExperiencePlayerSpawningManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(false);
	bAutoRegister = true;
	bAutoActivate = true;
	bWantsInitializeComponent = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bAllowTickOnDedicatedServer = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UExperiencePlayerSpawningManagerComponent::InitializeComponent()
{
	Super::InitializeComponent();

	// 关卡是流送进来的，所以除了遍历当前世界，还要监听后续加入的关卡
	FWorldDelegates::LevelAddedToWorld.AddUObject(this, &ThisClass::OnLevelAdded);

	UWorld* World = GetWorld();
	World->AddOnActorSpawnedHandler(FOnActorSpawned::FDelegate::CreateUObject(this, &ThisClass::HandleOnActorSpawned));

	for (TActorIterator<AExperiencePlayerStart> It(World); It; ++It)
	{
		if (AExperiencePlayerStart* PlayerStart = *It)
		{
			CachedPlayerStarts.Add(PlayerStart);
		}
	}
}

void UExperiencePlayerSpawningManagerComponent::OnLevelAdded(ULevel* InLevel, UWorld* InWorld)
{
	if (InWorld == GetWorld())
	{
		for (AActor* Actor : InLevel->Actors)
		{
			if (AExperiencePlayerStart* PlayerStart = Cast<AExperiencePlayerStart>(Actor))
			{
				ensure(!CachedPlayerStarts.Contains(PlayerStart));
				CachedPlayerStarts.Add(PlayerStart);
			}
		}
	}
}

void UExperiencePlayerSpawningManagerComponent::HandleOnActorSpawned(AActor* SpawnedActor)
{
	if (AExperiencePlayerStart* PlayerStart = Cast<AExperiencePlayerStart>(SpawnedActor))
	{
		CachedPlayerStarts.Add(PlayerStart);
	}
}

// AExperienceGameMode 转发过来的调用
// 之所以要转发，是为了让每一次重生（无论玩家还是 Bot）都能被本组件接管
//======================================================================

AActor* UExperiencePlayerSpawningManagerComponent::OnChoosePlayerStart(AController* Player, TArray<AExperiencePlayerStart*>& PlayerStarts)
{
	// 默认不做任何过滤，交给蓝图钩子或默认的随机选点逻辑处理
	return nullptr;
}

AActor* UExperiencePlayerSpawningManagerComponent::ChoosePlayerStart(AController* Player)
{
	if (Player == nullptr)
	{
		return nullptr;
	}

#if WITH_EDITOR
	// 编辑器下「从此处播放」优先
	if (APlayerStart* PlayerStart = FindPlayFromHereStart(Player))
	{
		return PlayerStart;
	}
#endif

	// 先整理出一份有效的出生点，顺手把失效的弱引用清掉
	TArray<AExperiencePlayerStart*> StarterPoints;
	for (auto StartIt = CachedPlayerStarts.CreateIterator(); StartIt; ++StartIt)
	{
		if (AExperiencePlayerStart* Start = (*StartIt).Get())
		{
			StarterPoints.Add(Start);
		}
		else
		{
			StartIt.RemoveCurrent();
		}
	}

	// 纯观战者随便给一个点即可，并且不占用它
	if (APlayerState* PlayerState = Player->GetPlayerState<APlayerState>())
	{
		if (PlayerState->IsOnlyASpectator())
		{
			if (!StarterPoints.IsEmpty())
			{
				return StarterPoints[FMath::RandRange(0, StarterPoints.Num() - 1)];
			}

			return nullptr;
		}
	}

	// 扩展点①：C++ 自定义选点
	AActor* PlayerStart = OnChoosePlayerStart(Player, StarterPoints);

	// 扩展点②：蓝图自定义选点
	if (PlayerStart == nullptr)
	{
		TArray<AActor*> CandidateActors;
		CandidateActors.Reserve(StarterPoints.Num());
		for (AExperiencePlayerStart* Start : StarterPoints)
		{
			CandidateActors.Add(Start);
		}

		PlayerStart = K2_OnChoosePlayerStart(Player, CandidateActors);
	}

	// 默认逻辑：随机取一个没被占用的点
	if (PlayerStart == nullptr)
	{
		PlayerStart = GetFirstRandomUnoccupiedPlayerStart(Player, StarterPoints);
	}

	if (PlayerStart == nullptr)
	{
		UE_LOG(LogExperienceSpawning, Warning, TEXT("未能为控制器 [%s] 找到可用的出生点，请检查场景中是否放置了 AExperiencePlayerStart。"), *GetNameSafe(Player));
		return nullptr;
	}

	// 选中的点声明占用，避免同一帧被别的控制器重复选中
	if (AExperiencePlayerStart* ExperienceStart = Cast<AExperiencePlayerStart>(PlayerStart))
	{
		ExperienceStart->TryClaim(Player);
	}

	return PlayerStart;
}

bool UExperiencePlayerSpawningManagerComponent::ControllerCanRestart(AController* Player)
{
	// 默认永远允许重生
	// 项目层可在此接入：复活冷却、回合制只在回合开始时重生、场次结束禁止重生等
	bool bCanRestart = true;

	return bCanRestart;
}

void UExperiencePlayerSpawningManagerComponent::FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation)
{
	OnFinishRestartPlayer(NewPlayer, StartRotation);
	K2_OnFinishRestartPlayer(NewPlayer, StartRotation);
}

//======================================================================

APlayerStart* UExperiencePlayerSpawningManagerComponent::GetFirstRandomUnoccupiedPlayerStart(AController* Controller, const TArray<AExperiencePlayerStart*>& StartPoints) const
{
	if (Controller)
	{
		TArray<AExperiencePlayerStart*> UnOccupiedStartPoints;
		TArray<AExperiencePlayerStart*> OccupiedStartPoints;

		for (AExperiencePlayerStart* StartPoint : StartPoints)
		{
			const EExperiencePlayerStartLocationOccupancy State = StartPoint->GetLocationOccupancy(Controller);

			switch (State)
			{
			case EExperiencePlayerStartLocationOccupancy::Empty:
				UnOccupiedStartPoints.Add(StartPoint);
				break;
			case EExperiencePlayerStartLocationOccupancy::Partial:
				OccupiedStartPoints.Add(StartPoint);
				break;
			default:
				break;
			}
		}

		if (UnOccupiedStartPoints.Num() > 0)
		{
			return UnOccupiedStartPoints[FMath::RandRange(0, UnOccupiedStartPoints.Num() - 1)];
		}
		else if (OccupiedStartPoints.Num() > 0)
		{
			return OccupiedStartPoints[FMath::RandRange(0, OccupiedStartPoints.Num() - 1)];
		}
	}

	return nullptr;
}

#if WITH_EDITOR
APlayerStart* UExperiencePlayerSpawningManagerComponent::FindPlayFromHereStart(AController* Player)
{
	// 「从此处播放」只对玩家控制器生效，Bot 一律走正常出生点
	if (Player->IsA<APlayerController>())
	{
		if (UWorld* World = GetWorld())
		{
			for (TActorIterator<APlayerStart> It(World); It; ++It)
			{
				if (APlayerStart* PlayerStart = *It)
				{
					if (PlayerStart->IsA<APlayerStartPIE>())
					{
						return PlayerStart;
					}
				}
			}
		}
	}

	return nullptr;
}
#endif
