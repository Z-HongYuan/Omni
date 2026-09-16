// Copyright © 2026 张鸿源. All Rights Reserved.

#include "Contra/ContraPlayerLifeComponent.h"
#include "Contra/ContraRoundComponent.h"

#include "Attributes/ExtHealthSet.h"
#include "Character/OmniCharacter.h"
#include "Component/ExtDeathComponent.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Gameplay/ExpPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "System/ExtAbilitySystemComponent.h"
#include "TimerManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ContraPlayerLifeComponent)

UContraPlayerLifeComponent::UContraPlayerLifeComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
}

void UContraPlayerLifeComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass, Lives);
	DOREPLIFETIME(ThisClass, ProtectionEndTime);
}

void UContraPlayerLifeComponent::BeginPlay()
{
	Super::BeginPlay();
	APlayerState* PS = GetPlayerState<APlayerState>();
	if (!PS || !PS->HasAuthority()) return;
	Lives = FMath::Max(StartingLives, 1);
	PS->OnPawnSet.AddDynamic(this, &ThisClass::PawnChanged);
	PawnChanged(PS, PS->GetPawn(), nullptr);
}

void UContraPlayerLifeComponent::PawnChanged(APlayerState* Player, APawn* NewPawn, APawn* OldPawn)
{
	if (BoundDeath.IsValid()) BoundDeath->OnDeathStarted.RemoveDynamic(this, &ThisClass::DeathStarted);
	BoundDeath.Reset();
	AOmniCharacter* Character = Cast<AOmniCharacter>(NewPawn);
	if (!Character || !Player->HasAuthority()) return;
	BoundDeath = Character->GetDeathComponent();
	BoundDeath->OnDeathStarted.AddUniqueDynamic(this, &ThisClass::DeathStarted);
	// PS 上的属性集跨命保留，因此每个新 Avatar 在此显式恢复生命。
	AExpPlayerState* PS = Cast<AExpPlayerState>(Player);
	UExtAbilitySystemComponent* ASC = PS ? PS->GetExtAbilitySystemComponent() : nullptr;
	if (ASC && ASC->GetSet<UExtHealthSet>())
	{
		ASC->ClearAbilityInput();
		ASC->SetNumericAttributeBase(UExtHealthSet::GetHealthAttribute(), ASC->GetNumericAttribute(UExtHealthSet::GetMaxHealthAttribute()));
	}
	ProtectionEndTime = GetWorld()->GetTimeSeconds() + ProtectionSeconds;
	GetOwner()->ForceNetUpdate();
}

bool UContraPlayerLifeComponent::IsProtected() const
{
	const AGameStateBase* GS = GetWorld() ? GetWorld()->GetGameState() : nullptr;
	return GS && GS->GetServerWorldTimeSeconds() < ProtectionEndTime;
}

void UContraPlayerLifeComponent::DeathStarted(AActor* PawnActor)
{
	APawn* Pawn = Cast<APawn>(PawnActor);
	if (!GetOwner()->HasAuthority() || !Pawn || Lives <= 0 || GetWorld()->GetTimerManager().IsTimerActive(RespawnTimer)) return;
	RespawnController = Pawn->GetController();
	--Lives;
	GetOwner()->ForceNetUpdate();
	GetWorld()->GetTimerManager().SetTimer(RespawnTimer, this, &ThisClass::Respawn, FMath::Max(RespawnDelay, 0.1f), false);
}

void UContraPlayerLifeComponent::Respawn()
{
	AController* Controller = RespawnController.Get();
	if (BoundDeath.IsValid())
	{
		BoundDeath->FinishDeath();
		BoundDeath->GetOwner()->Destroy();
	}
	RespawnController.Reset();
	// 旧 Pawn 的 EndPlay 完整注销 ASC 后才能创建新 Avatar。
	if (Lives > 0 && IsValid(Controller))
	{
		if (AGameModeBase* GM = GetWorld()->GetAuthGameMode()) GM->RestartPlayer(Controller);
	}
}

void UContraPlayerLifeComponent::ServerRequestRestart_Implementation()
{
	const AGameStateBase* GS = GetWorld()->GetGameState();
	if (UContraRoundComponent* Round = GS ? GS->FindComponentByClass<UContraRoundComponent>() : nullptr) Round->RestartRun();
}

void UContraPlayerLifeComponent::EndPlay(EEndPlayReason::Type Reason)
{
	GetWorld()->GetTimerManager().ClearTimer(RespawnTimer);
	if (APlayerState* PS = GetPlayerState<APlayerState>()) PS->OnPawnSet.RemoveDynamic(this, &ThisClass::PawnChanged);
	if (BoundDeath.IsValid()) BoundDeath->OnDeathStarted.RemoveDynamic(this, &ThisClass::DeathStarted);
	BoundDeath.Reset();
	RespawnController.Reset();
	Super::EndPlay(Reason);
}
