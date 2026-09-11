// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Components/ExpBotCreationComponent.h"

#include "Components/ExpManagerComponent.h"
#include "Components/ExpPawnExtensionComponent.h"
#include "Data/ExpDefinition.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Gameplay/ExpGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Logs/LogExpSystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ExpBotCreationComponent)

UExpBotCreationComponent::UExpBotCreationComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(false);
}

void UExpBotCreationComponent::BeginPlay()
{
	Super::BeginPlay();

	// 监听体验加载完成
	// 这里刻意使用低优先级：确保 GameMode 与 PlayerState 的 OnExperienceLoaded 先执行完
	// 这样 Bot 被创建时，体验里的 GameFeature 与默认 PawnData 都已经就绪
	AGameStateBase* GameState = GetGameStateChecked<AGameStateBase>();
	UExpManagerComponent* ExperienceComponent = GameState->FindComponentByClass<UExpManagerComponent>();
	check(ExperienceComponent);
	ExperienceComponent->CallOrRegister_OnExperienceLoaded_LowPriority(FExpLoaded::FDelegate::CreateUObject(this, &ThisClass::OnExperienceLoaded));
}

void UExpBotCreationComponent::OnExperienceLoaded(const UExpDefinition* Experience)
{
	// 只有权威端创建 Bot
	if (HasAuthority())
	{
		ServerCreateBots();
	}
}

int32 UExpBotCreationComponent::GetEffectiveBotCount() const
{
	int32 EffectiveBotCount = NumBotsToCreate;

	// 允许 URL 覆盖：?NumBots=8
	if (AGameModeBase* GameModeBase = GetGameMode<AGameModeBase>())
	{
		EffectiveBotCount = UGameplayStatics::GetIntOption(GameModeBase->OptionsString, TEXT("NumBots"), EffectiveBotCount);
	}

	return EffectiveBotCount;
}

void UExpBotCreationComponent::ServerCreateBots_Implementation()
{
	if (BotControllerClass == nullptr)
	{
		UE_LOG(LogExpSystem, Warning, TEXT("BotCreationComponent 未配置 BotControllerClass，无法创建 Bot。"));
		return;
	}

	RemainingBotNames = RandomBotNames;

	const int32 EffectiveBotCount = GetEffectiveBotCount();
	for (int32 Count = 0; Count < EffectiveBotCount; ++Count)
	{
		SpawnOneBot();
	}
}

FString UExpBotCreationComponent::CreateBotName(int32 PlayerIndex)
{
	FString Result;
	if (RemainingBotNames.Num() > 0)
	{
		const int32 NameIndex = FMath::RandRange(0, RemainingBotNames.Num() - 1);
		Result = RemainingBotNames[NameIndex];
		RemainingBotNames.RemoveAtSwap(NameIndex);
	}
	else
	{
		//@TODO: 目前 PlayerId 只有玩家才会被初始化，Bot 这里先随机一个
		PlayerIndex = FMath::RandRange(260, 260 + 100);
		Result = FString::Printf(TEXT("Bot %d"), PlayerIndex);
	}
	return Result;
}

void UExpBotCreationComponent::SpawnOneBot()
{
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnInfo.OverrideLevel = GetComponentLevel();
	SpawnInfo.ObjectFlags |= RF_Transient;

	AExpAIController* NewController = GetWorld()->SpawnActor<AExpAIController>(BotControllerClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnInfo);
	if (NewController == nullptr)
	{
		return;
	}

	AExpGameMode* GameMode = GetGameMode<AExpGameMode>();
	check(GameMode);

	// 起个名字
	if (NewController->PlayerState != nullptr)
	{
		NewController->PlayerState->SetPlayerName(CreateBotName(NewController->PlayerState->GetPlayerId()));
	}

	// 与玩家完全一致的处理链条：先广播初始化完成，再走 GameMode 的重生流程
	GameMode->GenericPlayerInitialization(NewController);
	GameMode->RestartPlayer(NewController);

	// 重生完成后推一次组件状态链，确保 Pawn 上的组件能往下走
	if (APawn* NewPawn = NewController->GetPawn())
	{
		if (UExpPawnExtensionComponent* PawnExtComponent = UExpPawnExtensionComponent::FindPawnExtensionComponent(NewPawn))
		{
			PawnExtComponent->CheckDefaultInitialization();
		}
	}

	SpawnedBotList.Add(NewController);

	// 交给项目层做自定义初始化（队伍、难度、外观等）
	K2_OnBotSpawned(NewController);
}

void UExpBotCreationComponent::RemoveOneBot()
{
	if (SpawnedBotList.Num() > 0)
	{
		// 目前随机移除一个，因为默认的 Bot 都一样
		// 项目层可重载此函数，按技能、分数等标准挑选要移除的 Bot
		const int32 BotToRemoveIndex = FMath::RandRange(0, SpawnedBotList.Num() - 1);

		AExpAIController* BotToRemove = SpawnedBotList[BotToRemoveIndex];
		SpawnedBotList.RemoveAtSwap(BotToRemoveIndex);

		if (BotToRemove)
		{
			// 先销毁 Pawn，再销毁 Controller（销毁 Controller 会触发 Logout 等流程）
			// 注意：如果项目层有死亡表现，请重载此函数，改为先触发死亡再销毁
			if (APawn* ControlledPawn = BotToRemove->GetPawn())
			{
				ControlledPawn->Destroy();
			}

			BotToRemove->Destroy();
		}
	}
}
