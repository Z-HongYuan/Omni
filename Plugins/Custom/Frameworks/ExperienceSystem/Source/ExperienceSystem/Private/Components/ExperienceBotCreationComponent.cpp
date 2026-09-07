// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Components/ExperienceBotCreationComponent.h"

#include "Components/ExperienceManagerComponent.h"
#include "Components/ExperiencePawnExtensionComponent.h"
#include "Data/ExperienceDefinition.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Gameplay/ExperienceGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Logs/LogExperienceSystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ExperienceBotCreationComponent)

UExperienceBotCreationComponent::UExperienceBotCreationComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(false);
}

void UExperienceBotCreationComponent::BeginPlay()
{
	Super::BeginPlay();

	// 监听体验加载完成
	// 这里刻意使用低优先级：确保 GameMode 与 PlayerState 的 OnExperienceLoaded 先执行完
	// 这样 Bot 被创建时，体验里的 GameFeature 与默认 PawnData 都已经就绪
	AGameStateBase* GameState = GetGameStateChecked<AGameStateBase>();
	UExperienceManagerComponent* ExperienceComponent = GameState->FindComponentByClass<UExperienceManagerComponent>();
	check(ExperienceComponent);
	ExperienceComponent->CallOrRegister_OnExperienceLoaded_LowPriority(FOnExperienceLoaded::FDelegate::CreateUObject(this, &ThisClass::OnExperienceLoaded));
}

void UExperienceBotCreationComponent::OnExperienceLoaded(const UExperienceDefinition* Experience)
{
	// 只有权威端创建 Bot
	if (HasAuthority())
	{
		ServerCreateBots();
	}
}

int32 UExperienceBotCreationComponent::GetEffectiveBotCount() const
{
	int32 EffectiveBotCount = NumBotsToCreate;

	// 允许 URL 覆盖：?NumBots=8
	if (AGameModeBase* GameModeBase = GetGameMode<AGameModeBase>())
	{
		EffectiveBotCount = UGameplayStatics::GetIntOption(GameModeBase->OptionsString, TEXT("NumBots"), EffectiveBotCount);
	}

	return EffectiveBotCount;
}

void UExperienceBotCreationComponent::ServerCreateBots_Implementation()
{
	if (BotControllerClass == nullptr)
	{
		UE_LOG(LogExperienceSystem, Warning, TEXT("BotCreationComponent 未配置 BotControllerClass，无法创建 Bot。"));
		return;
	}

	RemainingBotNames = RandomBotNames;

	const int32 EffectiveBotCount = GetEffectiveBotCount();
	for (int32 Count = 0; Count < EffectiveBotCount; ++Count)
	{
		SpawnOneBot();
	}
}

FString UExperienceBotCreationComponent::CreateBotName(int32 PlayerIndex)
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

void UExperienceBotCreationComponent::SpawnOneBot()
{
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnInfo.OverrideLevel = GetComponentLevel();
	SpawnInfo.ObjectFlags |= RF_Transient;

	AExperienceAIController* NewController = GetWorld()->SpawnActor<AExperienceAIController>(BotControllerClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnInfo);
	if (NewController == nullptr)
	{
		return;
	}

	AExperienceGameMode* GameMode = GetGameMode<AExperienceGameMode>();
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
		if (UExperiencePawnExtensionComponent* PawnExtComponent = UExperiencePawnExtensionComponent::FindPawnExtensionComponent(NewPawn))
		{
			PawnExtComponent->CheckDefaultInitialization();
		}
	}

	SpawnedBotList.Add(NewController);

	// 交给项目层做自定义初始化（队伍、难度、外观等）
	K2_OnBotSpawned(NewController);
}

void UExperienceBotCreationComponent::RemoveOneBot()
{
	if (SpawnedBotList.Num() > 0)
	{
		// 目前随机移除一个，因为默认的 Bot 都一样
		// 项目层可重载此函数，按技能、分数等标准挑选要移除的 Bot
		const int32 BotToRemoveIndex = FMath::RandRange(0, SpawnedBotList.Num() - 1);

		AExperienceAIController* BotToRemove = SpawnedBotList[BotToRemoveIndex];
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
