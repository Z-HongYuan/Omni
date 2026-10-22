// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Components/ExpPlayerSpawningManagerComponent.h"

#include "Engine/PlayerStartPIE.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerState.h"
#include "Gameplay/ExpPlayerStart.h"
#include "Logs/LogExpSystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ExpPlayerSpawningManagerComponent)

UExpPlayerSpawningManagerComponent::UExpPlayerSpawningManagerComponent(const FObjectInitializer& ObjectInitializer)
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

void UExpPlayerSpawningManagerComponent::InitializeComponent()
{
	Super::InitializeComponent();

	// 同时监听后续流入的关卡和新生成 Actor，补充初始遍历未覆盖的出生点。
	FWorldDelegates::LevelAddedToWorld.AddUObject(this, &ThisClass::OnLevelAdded);

	UWorld* World = GetWorld();
	World->AddOnActorSpawnedHandler(FOnActorSpawned::FDelegate::CreateUObject(this, &ThisClass::HandleOnActorSpawned));

	for (TActorIterator<AExpPlayerStart> It(World); It; ++It)
	{
		if (AExpPlayerStart* PlayerStart = *It)
		{
			CachedPlayerStarts.Add(PlayerStart);
		}
	}
}

void UExpPlayerSpawningManagerComponent::OnLevelAdded(ULevel* InLevel, UWorld* InWorld)
{
	if (InWorld == GetWorld())
	{
		for (AActor* Actor : InLevel->Actors)
		{
			if (AExpPlayerStart* PlayerStart = Cast<AExpPlayerStart>(Actor))
			{
				ensure(!CachedPlayerStarts.Contains(PlayerStart));
				CachedPlayerStarts.Add(PlayerStart);
			}
		}
	}
}

void UExpPlayerSpawningManagerComponent::HandleOnActorSpawned(AActor* SpawnedActor)
{
	if (AExpPlayerStart* PlayerStart = Cast<AExpPlayerStart>(SpawnedActor))
	{
		CachedPlayerStarts.Add(PlayerStart);
	}
}


AActor* UExpPlayerSpawningManagerComponent::OnChoosePlayerStart(AController* Player, TArray<AExpPlayerStart*>& PlayerStarts)
{
	// 默认交给蓝图钩子或随机选点策略。
	return nullptr;
}

AActor* UExpPlayerSpawningManagerComponent::ChoosePlayerStart(AController* Player)
{
	if (Player == nullptr)
	{
		return nullptr;
	}

#if WITH_EDITOR
	// 编辑器的“从此处播放”优先于普通出生点。
	if (APlayerStart* PlayerStart = FindPlayFromHereStart(Player))
	{
		return PlayerStart;
	}
#endif

	// 收集有效出生点，并从缓存移除失效弱引用。
	TArray<AExpPlayerStart*> StarterPoints;
	for (auto StartIt = CachedPlayerStarts.CreateIterator(); StartIt; ++StartIt)
	{
		if (AExpPlayerStart* Start = (*StartIt).Get())
		{
			StarterPoints.Add(Start);
		}
		else
		{
			StartIt.RemoveCurrent();
		}
	}

	// 纯观战者随机选点，不声明占用。
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

	// 优先调用 C++ 选点钩子。
	AActor* PlayerStart = OnChoosePlayerStart(Player, StarterPoints);

	// C++ 未选中时尝试蓝图钩子。
	if (PlayerStart == nullptr)
	{
		TArray<AActor*> CandidateActors;
		CandidateActors.Reserve(StarterPoints.Num());
		for (AExpPlayerStart* Start : StarterPoints)
		{
			CandidateActors.Add(Start);
		}

		PlayerStart = K2_OnChoosePlayerStart(Player, CandidateActors);
	}

	// 最后按碰撞占用状态随机选点。
	if (PlayerStart == nullptr)
	{
		PlayerStart = GetFirstRandomUnoccupiedPlayerStart(Player, StarterPoints);
	}

	if (PlayerStart == nullptr)
	{
		UE_LOG(LogExpSpawning, Warning, TEXT("未能为控制器 [%s] 找到可用的出生点，请检查场景中是否放置了 AExpPlayerStart。"), *GetNameSafe(Player));
		return nullptr;
	}

	// 尝试记录声明占用；当前默认选点不检查该标记，且此处不因声明失败重新选点。
	if (AExpPlayerStart* ExperienceStart = Cast<AExpPlayerStart>(PlayerStart))
	{
		ExperienceStart->TryClaim(Player);
	}

	return PlayerStart;
}

bool UExpPlayerSpawningManagerComponent::ControllerCanRestart(AController* Player)
{
	// 当前不施加额外重生限制。
	bool bCanRestart = true;

	return bCanRestart;
}

void UExpPlayerSpawningManagerComponent::FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation)
{
	OnFinishRestartPlayer(NewPlayer, StartRotation);
	K2_OnFinishRestartPlayer(NewPlayer, StartRotation);
}


APlayerStart* UExpPlayerSpawningManagerComponent::GetFirstRandomUnoccupiedPlayerStart(AController* Controller, const TArray<AExpPlayerStart*>& StartPoints) const
{
	if (Controller)
	{
		TArray<AExpPlayerStart*> UnOccupiedStartPoints;
		TArray<AExpPlayerStart*> OccupiedStartPoints;

		for (AExpPlayerStart* StartPoint : StartPoints)
		{
			const EExpPlayerStartLocationOccupancy State = StartPoint->GetLocationOccupancy(Controller);

			switch (State)
			{
			case EExpPlayerStartLocationOccupancy::Empty:
				UnOccupiedStartPoints.Add(StartPoint);
				break;
			case EExpPlayerStartLocationOccupancy::Partial:
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
APlayerStart* UExpPlayerSpawningManagerComponent::FindPlayFromHereStart(AController* Player)
{
	// “从此处播放”只对玩家控制器生效，Bot 使用普通出生点。
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
