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

	// 使用低优先级通知，等待 GameMode 和 PlayerState 的普通就绪回调完成后创建 Bot。
	AGameStateBase* GameState = GetGameStateChecked<AGameStateBase>();
	UExpManagerComponent* ExperienceComponent = GameState->FindComponentByClass<UExpManagerComponent>();
	check(ExperienceComponent);
	ExperienceComponent->CallOrRegister_OnExperienceLoaded_LowPriority(FExpLoaded::FDelegate::CreateUObject(this, &ThisClass::OnExperienceLoaded));
}

void UExpBotCreationComponent::OnExperienceLoaded(const UExpDefinition* Experience)
{
	// 仅权威端自动创建 Bot。
	if (HasAuthority())
	{
		ServerCreateBots();
	}
}

int32 UExpBotCreationComponent::GetEffectiveBotCount() const
{
	int32 EffectiveBotCount = NumBotsToCreate;

	// URL 的 NumBots 选项可覆盖配置数量。
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

	// 为拥有 PlayerState 的 Bot 设置名称。
	if (NewController->PlayerState != nullptr)
	{
		NewController->PlayerState->SetPlayerName(CreateBotName(NewController->PlayerState->GetPlayerId()));
	}

	// 先执行通用初始化并广播，再通过 GameMode 请求重生。
	GameMode->GenericPlayerInitialization(NewController);
	GameMode->RestartPlayer(NewController);

	// 如果重生得到 Pawn，再推动其组件初始化状态链。
	if (APawn* NewPawn = NewController->GetPawn())
	{
		if (UExpPawnExtensionComponent* PawnExtComponent = UExpPawnExtensionComponent::FindPawnExtensionComponent(NewPawn))
		{
			PawnExtComponent->CheckDefaultInitialization();
		}
	}

	SpawnedBotList.Add(NewController);

	// 通知项目层进行自定义初始化，此时重生可能尚未成功。
	K2_OnBotSpawned(NewController);
}

void UExpBotCreationComponent::RemoveOneBot()
{
	if (SpawnedBotList.Num() > 0)
	{
		// 默认随机选择移除对象，派生类可替换选择策略。
		const int32 BotToRemoveIndex = FMath::RandRange(0, SpawnedBotList.Num() - 1);

		AExpAIController* BotToRemove = SpawnedBotList[BotToRemoveIndex];
		SpawnedBotList.RemoveAtSwap(BotToRemoveIndex);

		if (BotToRemove)
		{
			// 直接销毁 Pawn 后再销毁控制器；需要死亡表现时由派生类重写此流程。
			if (APawn* ControlledPawn = BotToRemove->GetPawn())
			{
				ControlledPawn->Destroy();
			}

			BotToRemove->Destroy();
		}
	}
}
