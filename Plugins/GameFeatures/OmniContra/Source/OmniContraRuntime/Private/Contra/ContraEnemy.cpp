// Copyright © 2026 张鸿源. All Rights Reserved.

#include "Contra/ContraEnemy.h"

#include "Attributes/ExtHealthSet.h"
#include "Character/OmniCharacter.h"
#include "Component/ExtDeathComponent.h"
#include "Component/ExtHealthComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Contra/ContraPlayerLifeComponent.h"
#include "Contra/ContraRoundComponent.h"
#include "GameFramework/GameStateBase.h"
#include "DrawDebugHelpers.h"
#include "EngineUtils.h"
#include "ExtHealthLibrary.h"
#include "GameFramework/PlayerState.h"
#include "System/ExtAbilitySystemComponent.h"
#include "TimerManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ContraEnemy)

AContraEnemy::AContraEnemy()
{
	bReplicates = true;
	Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
	SetRootComponent(Body);
	Body->SetCollisionProfileName(TEXT("BlockAll"));
	AbilitySystem = CreateDefaultSubobject<UExtAbilitySystemComponent>(TEXT("AbilitySystem"));
	AbilitySystem->SetIsReplicated(true);
	AbilitySystem->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	HealthSet = CreateDefaultSubobject<UExtHealthSet>(TEXT("HealthSet"));
	Health = CreateDefaultSubobject<UExtHealthComponent>(TEXT("Health"));
	Death = CreateDefaultSubobject<UExtDeathComponent>(TEXT("Death"));
}

UAbilitySystemComponent* AContraEnemy::GetAbilitySystemComponent() const { return AbilitySystem; }
bool AContraEnemy::IsDead() const { return Death->IsDeadOrDying(); }

void AContraEnemy::BeginPlay()
{
	Super::BeginPlay();
	AbilitySystem->InitAbilityActorInfo(this, this);
	Health->OnOutOfHealth.AddDynamic(this, &ThisClass::OutOfHealth);
	Death->OnDeathStarted.AddDynamic(this, &ThisClass::DeathStarted);
	Health->InitializeWithAbilitySystem(AbilitySystem);
	Death->InitializeWithAbilitySystem(AbilitySystem);
	if (HasAuthority()) GetWorldTimerManager().SetTimer(AttackTimer, this, &ThisClass::Attack, FMath::Max(AttackInterval, 0.2f), true);
}

void AContraEnemy::Attack()
{
	if (IsDead()) return;
	const AGameStateBase* GS = GetWorld()->GetGameState();
	const UContraRoundComponent* Round = GS ? GS->FindComponentByClass<UContraRoundComponent>() : nullptr;
	if (Round && Round->GetPhase() != EContraRoundPhase::Playing) return;
	AOmniCharacter* Target = nullptr;
	double BestDistance = FMath::Square(AttackRange);
	for (TActorIterator<AOmniCharacter> It(GetWorld()); It; ++It)
	{
		if (!It->GetController() || It->GetDeathComponent()->IsDeadOrDying()) continue;
		const double Distance = FVector::DistSquared(GetActorLocation(), It->GetActorLocation());
		if (Distance < BestDistance)
		{
			Target = *It;
			BestDistance = Distance;
		}
	}
	if (!Target) return;
	const APlayerState* PS = Target->GetPlayerState();
	const UContraPlayerLifeComponent* Life = PS ? PS->FindComponentByClass<UContraPlayerLifeComponent>() : nullptr;
	if (Life && Life->IsProtected()) return;
	FHitResult Hit;
	FCollisionQueryParams Query(SCENE_QUERY_STAT(ContraEnemy), false, this);
	const FVector End = Target->GetActorLocation();
	if (GetWorld()->LineTraceSingleByChannel(Hit, GetActorLocation(), End, ECC_Visibility, Query) && Hit.GetActor() == Target)
	{
		UExtHealthLibrary::ApplyDamage(AbilitySystem, Target->GetExtAbilitySystemComponent(), AttackDamage, Hit);
		ShowShot(Hit.ImpactPoint);
	}
}

void AContraEnemy::OutOfHealth(UExtHealthComponent* Component, float OldValue, float NewValue, AActor* InstigatorActor) { Death->StartDeath(); }

void AContraEnemy::DeathStarted(AActor* OwnerActor)
{
	Body->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetActorHiddenInGame(true);
	GetWorldTimerManager().ClearTimer(AttackTimer);
	if (HasAuthority())
	{
		Death->FinishDeath();
		SetLifeSpan(0.5f);
	}
}

void AContraEnemy::ShowShot_Implementation(FVector_NetQuantize Target)
{
	if (GetNetMode() != NM_DedicatedServer) DrawDebugLine(GetWorld(), GetActorLocation(), Target, FColor::Red, false, 0.18f, 0, 3.0f);
}

void AContraEnemy::EndPlay(EEndPlayReason::Type Reason)
{
	GetWorldTimerManager().ClearTimer(AttackTimer);
	Health->UninitializeFromAbilitySystem();
	Death->UninitializeFromAbilitySystem();
	AbilitySystem->ClearActorInfo();
	Super::EndPlay(Reason);
}
