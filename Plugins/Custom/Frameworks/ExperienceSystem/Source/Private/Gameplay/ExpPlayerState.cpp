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

	// 初始化时注册 OnExperienceLoaded 事件
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

	// 使组件尝试进入下一步
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
	// 检查 PawnData, 并且只能在权威中设置,且仅能设置一次
	check(InPawnData);
	if (GetLocalRole() != ROLE_Authority) { return; }
	if (PawnData)
	{
		UE_LOG(LogExpSystemPawnData, Error, TEXT("Trying to set PawnData [%s] on player state [%s] that already has valid PawnData [%s]."), *GetNameSafe(InPawnData), *GetNameSafe(this), *GetNameSafe(PawnData));
		return;
	}

	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, PawnData, this);
	PawnData = InPawnData;

	// 给予能力集
	for (const UExtAbilitySet* AbilitySet : PawnData->AbilitySets)
	{
		if (AbilitySet) AbilitySet->GiveToAbilitySystem(AbilitySystemComponent, nullptr);
	}

	// 广播能力集已就绪
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(this, NAME_GiveCustomAbilityReady);

	// 强制更新一次
	ForceNetUpdate();
}

void AExpPlayerState::OnExperienceLoaded(const class UExpDefinition* CurrentExperience)
{
	// GS加载Exp,然后根据Exp设置PawnData

	// check(CurrentExperience->DefaultPawnData);
	//
	// if (CurrentExperience->DefaultPawnData)
	// 	SetPawnData(CurrentExperience->DefaultPawnData);

	// 统一使用GameMode获取PawnData, 而不是直接使用Experience的DefaultPawnData,因为可能对PawnData有自定义处理
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
