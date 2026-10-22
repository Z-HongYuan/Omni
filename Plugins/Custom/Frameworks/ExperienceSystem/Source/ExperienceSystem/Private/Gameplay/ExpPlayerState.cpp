// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Gameplay/ExpPlayerState.h"

#include "Components/ExpManagerComponent.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Components/ExpPawnExtensionComponent.h"
#include "Data/ExtAbilitySet.h"
#include "Data/ExpPawnData.h"
#include "Gameplay/ExpGameMode.h"
#include "Logs/LogExpSystem.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"
#include "System/ExtAbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ExpPlayerState)

AExpPlayerState::AExpPlayerState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetNetUpdateFrequency(100.0f);

	AbilitySystemComponent = ObjectInitializer.CreateDefaultSubobject<UExtAbilitySystemComponent>(this, TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
}

void AExpPlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, PawnData, SharedParams);
}

void AExpPlayerState::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	check(AbilitySystemComponent);
	AbilitySystemComponent->InitAbilityActorInfo(this, GetPawn());

	// 仅服务器游戏世界注册体验就绪通知，以授予 PawnData 中的能力。
	UWorld* World = GetWorld();
	if (World && World->IsGameWorld() && World->GetNetMode() != NM_Client)
	{
		AGameStateBase* GameState = GetWorld()->GetGameState();
		check(GameState);
		UExpManagerComponent* ExperienceComponent = GameState->FindComponentByClass<UExpManagerComponent>();
		check(ExperienceComponent);
		ExperienceComponent->CallOrRegister_OnExperienceLoaded(FExpLoaded::FDelegate::CreateUObject(this, &ThisClass::OnExperienceLoaded));
	}
	// UExpManagerComponent* ExperienceComponent = UExperienceSystemHelper::GetExperienceManagerComponentForContext(this);
	// check(ExperienceComponent);
	// ExperienceComponent->CallOrRegister_OnExperienceLoaded(FExpLoaded::FDelegate::CreateUObject(this, &ThisClass::OnExperienceLoaded));
}

void AExpPlayerState::ClientInitialize(class AController* C)
{
	Super::ClientInitialize(C);

	// PlayerState 客户端初始化后，重新检查 Pawn 的初始化条件。
	if (UExpPawnExtensionComponent* PawnExtComp = UExpPawnExtensionComponent::FindPawnExtensionComponent(GetPawn()))
	{
		PawnExtComp->CheckDefaultInitialization();
	}
}

UExtAbilitySystemComponent* AExpPlayerState::GetExtAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UAbilitySystemComponent* AExpPlayerState::GetAbilitySystemComponent() const
{
	return GetExtAbilitySystemComponent();
}

const FName AExpPlayerState::NAME_GiveCustomAbilityReady("GiveCustomAbilitiesReady");

void AExpPlayerState::SetPawnData(const UExpPawnData* InPawnData)
{
	// 只允许权威端写入一次 PawnData，避免重复授予能力。
	check(InPawnData);
	if (GetLocalRole() != ROLE_Authority) { return; }
	if (PawnData)
	{
		UE_LOG(LogExpSystemPawnData, Error, TEXT("Trying to set PawnData [%s] on player state [%s] that already has valid PawnData [%s]."), *GetNameSafe(InPawnData), *GetNameSafe(this), *GetNameSafe(PawnData));
		return;
	}

	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, PawnData, this);
	PawnData = InPawnData;

	// 将配置中的能力集授予玩家 ASC。
	for (const UExtAbilitySet* AbilitySet : PawnData->AbilitySets)
	{
		if (AbilitySet) AbilitySet->GiveToAbilitySystem(AbilitySystemComponent, nullptr);
	}

	// 通知模块化功能：能力集已授予。
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(this, NAME_GiveCustomAbilityReady);

	// 请求尽快复制新 PawnData。
	ForceNetUpdate();
}

void AExpPlayerState::OnExperienceLoaded(const class UExpDefinition* CurrentExperience)
{
	// 体验就绪后通过 GameMode 解析玩家配置。

	// check(CurrentExperience->DefaultPawnData);
	//
	// if (CurrentExperience->DefaultPawnData)
	// 	SetPawnData(CurrentExperience->DefaultPawnData);

	// 通过 GameMode 取 PawnData，保留项目自定义的选择逻辑。
	if (AExpGameMode* CustomGameMode = GetWorld()->GetAuthGameMode<AExpGameMode>())
	{
		if (const UExpPawnData* NewPawnData = CustomGameMode->GetPawnDataForController(GetOwningController()))
		{
			SetPawnData(NewPawnData);
		}
		else
		{
			UE_LOG(LogExpSystemPawnData, Error, TEXT("CustomPlayerState::OnExperienceLoaded(): Unable to find PawnData to initialize player state [%s]!"), *GetNameSafe(this));
		}
	}
}
