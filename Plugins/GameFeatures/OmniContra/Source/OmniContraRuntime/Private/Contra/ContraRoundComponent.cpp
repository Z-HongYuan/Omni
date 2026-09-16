// Copyright © 2026 张鸿源. All Rights Reserved.

#include "Contra/ContraRoundComponent.h"

#include "Character/OmniCharacter.h"
#include "Component/ExtDeathComponent.h"
#include "Contra/ContraEnemy.h"
#include "Contra/ContraPlayerLifeComponent.h"
#include "EngineUtils.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ContraRoundComponent)

UContraRoundComponent::UContraRoundComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.2f;
}

void UContraRoundComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass, Phase);
}

void UContraRoundComponent::TickComponent(float DeltaTime, ELevelTick Type, FActorComponentTickFunction* Function)
{
	Super::TickComponent(DeltaTime, Type, Function);
	if (!GetOwner()->HasAuthority() || Phase != EContraRoundPhase::Playing) return;
	int32 Players = 0, Remaining = 0;
	for (APlayerState* PS : GetGameState<AGameStateBase>()->PlayerArray)
	{
		if (PS->IsOnlyASpectator()) continue;
		if (const UContraPlayerLifeComponent* Life = PS->FindComponentByClass<UContraPlayerLifeComponent>())
		{
			++Players;
			Remaining += Life->GetLives();
		}
	}
	if (Players == 0) return;
	if (Remaining == 0) Phase = EContraRoundPhase::Lost;
	else
	{
		for (TActorIterator<AContraEnemy> It(GetWorld()); It; ++It) if (!It->IsDead()) return;
		for (TActorIterator<AOmniCharacter> It(GetWorld()); It; ++It)
		{
			if (It->GetController() && !It->GetDeathComponent()->IsDeadOrDying() && It->GetActorLocation().X >= GoalX)
			{
				Phase = EContraRoundPhase::Won;
				break;
			}
		}
	}
	if (Phase != EContraRoundPhase::Playing) GetOwner()->ForceNetUpdate();
}

void UContraRoundComponent::RestartRun()
{
	if (!GetOwner()->HasAuthority() || Phase == EContraRoundPhase::Playing || bTravelRequested) return;
	bTravelRequested = GetWorld()->ServerTravel(UWorld::RemovePIEPrefix(GetWorld()->GetOutermost()->GetName()));
}
