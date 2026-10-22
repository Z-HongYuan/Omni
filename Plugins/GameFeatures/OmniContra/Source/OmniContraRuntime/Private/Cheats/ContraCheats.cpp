// Copyright © 2026 张鸿源. All Rights Reserved.

#include "Cheats/ContraCheats.h"

#include "Component/ExtHealthComponent.h"
#include "GameFramework/CheatManagerDefines.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ContraCheats)

void UContraCheats::KillPlayer()
{
#if UE_WITH_CHEAT_MANAGER
	APlayerController* PlayerController = GetPlayerController();
	APawn* Pawn = PlayerController ? PlayerController->GetPawn() : nullptr;
	if (!Pawn || !Pawn->HasAuthority()) return;

	if (UExtHealthComponent* HealthComponent = UExtHealthComponent::FindHealthComponent(Pawn))
	{
		HealthComponent->DamageSelfDestruct();
	}
#endif
}
