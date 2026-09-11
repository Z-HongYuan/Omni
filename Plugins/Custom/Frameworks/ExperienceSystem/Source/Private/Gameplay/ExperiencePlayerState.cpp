// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Gameplay/ExperiencePlayerState.h"

#include "Components/ExperienceManagerComponent.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Components/ExperiencePawnExtensionComponent.h"
#include "Data/CustomAbilitySet.h"
#include "Data/ExperiencePawnData.h"
#include "Gameplay/ExperienceGameMode.h"
#include "Logs/LogExperienceSystem.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"
#include "System/CustomAbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ExperiencePlayerState)

AExperiencePlayerState::AExperiencePlayerState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetNetUpdateFrequency(100.0f);

	AbilitySystemComponent = ObjectInitializer.CreateDefaultSubobject<UCustomAbilitySystemComponent>(this, TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
}

void AExperiencePlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, PawnData, SharedParams);
}

void AExperiencePlayerState::PostInitializeComponents()
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
		UExperienceManagerComponent* ExperienceComponent = GameState->FindComponentByClass<UExperienceManagerComponent>();
		check(ExperienceComponent);
		ExperienceComponent->CallOrRegister_OnExperienceLoaded(FOnExperienceLoaded::FDelegate::CreateUObject(this, &ThisClass::OnExperienceLoaded));
	}
	// UExperienceManagerComponent* ExperienceComponent = UExperienceSystemHelper::GetExperienceManagerComponentForContext(this);
	// check(ExperienceComponent);
	// ExperienceComponent->CallOrRegister_OnExperienceLoaded(FOnExperienceLoaded::FDelegate::CreateUObject(this, &ThisClass::OnExperienceLoaded));
}

void AExperiencePlayerState::ClientInitialize(class AController* C)
{
	Super::ClientInitialize(C);

	// 使组件尝试进入下一步
	if (UExperiencePawnExtensionComponent* PawnExtComp = UExperiencePawnExtensionComponent::FindPawnExtensionComponent(GetPawn()))
	{
		PawnExtComp->CheckDefaultInitialization();
	}
}

UCustomAbilitySystemComponent* AExperiencePlayerState::GetCustomAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UAbilitySystemComponent* AExperiencePlayerState::GetAbilitySystemComponent() const
{
	return GetCustomAbilitySystemComponent();
}

const FName AExperiencePlayerState::NAME_GiveCustomAbilityReady("GiveCustomAbilitiesReady");

void AExperiencePlayerState::SetPawnData(const UExperiencePawnData* InPawnData)
{
	// 检查 PawnData, 并且只能在权威中设置,且仅能设置一次
	check(InPawnData);
	if (GetLocalRole() != ROLE_Authority) { return; }
	if (PawnData)
	{
		UE_LOG(LogExperienceSystemPawnData, Error, TEXT("Trying to set PawnData [%s] on player state [%s] that already has valid PawnData [%s]."), *GetNameSafe(InPawnData), *GetNameSafe(this), *GetNameSafe(PawnData));
		return;
	}

	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, PawnData, this);
	PawnData = InPawnData;

	// 给予能力集
	for (const UCustomAbilitySet* AbilitySet : PawnData->AbilitySets)
	{
		if (AbilitySet) AbilitySet->GiveToAbilitySystem(AbilitySystemComponent, nullptr);
	}

	// 广播能力集已就绪
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(this, NAME_GiveCustomAbilityReady);

	// 强制更新一次
	ForceNetUpdate();
}

void AExperiencePlayerState::OnExperienceLoaded(const class UExperienceDefinition* CurrentExperience)
{
	// GS加载Exp,然后根据Exp设置PawnData

	// check(CurrentExperience->DefaultPawnData);
	//
	// if (CurrentExperience->DefaultPawnData)
	// 	SetPawnData(CurrentExperience->DefaultPawnData);

	// 统一使用GameMode获取PawnData, 而不是直接使用Experience的DefaultPawnData,因为可能对PawnData有自定义处理
	if (AExperienceGameMode* CustomGameMode = GetWorld()->GetAuthGameMode<AExperienceGameMode>())
	{
		if (const UExperiencePawnData* NewPawnData = CustomGameMode->GetPawnDataForController(GetOwningController()))
		{
			SetPawnData(NewPawnData);
		}
		else
		{
			UE_LOG(LogExperienceSystemPawnData, Error, TEXT("CustomPlayerState::OnExperienceLoaded(): Unable to find PawnData to initialize player state [%s]!"), *GetNameSafe(this));
		}
	}
}
