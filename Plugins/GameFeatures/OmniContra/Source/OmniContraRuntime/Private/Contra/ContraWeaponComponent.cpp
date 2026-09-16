// Copyright © 2026 张鸿源. All Rights Reserved.

#include "Contra/ContraWeaponComponent.h"

#include "AbilitySystemGlobals.h"
#include "Character/OmniCharacter.h"
#include "Component/ExtDeathComponent.h"
#include "Contra/ContraEnemy.h"
#include "Contra/ContraRoundComponent.h"
#include "GameFramework/GameStateBase.h"
#include "DrawDebugHelpers.h"
#include "ExtHealthLibrary.h"
#include "System/ExtAbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ContraWeaponComponent)

UContraWeaponComponent::UContraWeaponComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
}

bool UContraWeaponComponent::TryFire()
{
	AOmniCharacter* Pawn = GetPawn<AOmniCharacter>();
	if (!Pawn || !Pawn->HasAuthority() || !Pawn->GetController() || Pawn->GetDeathComponent()->IsDeadOrDying()) return false;
	const AGameStateBase* GS = GetWorld()->GetGameState();
	const UContraRoundComponent* Round = GS ? GS->FindComponentByClass<UContraRoundComponent>() : nullptr;
	if (Round && Round->GetPhase() != EContraRoundPhase::Playing) return false;
	const double Now = GetWorld()->GetTimeSeconds();
	if (Now + UE_SMALL_NUMBER < NextFireTime) return false;
	NextFireTime = Now + GetFireInterval();

	FVector Direction = Pawn->GetController()->GetControlRotation().Vector();
	Direction.Y = 0.0;
	if (!Direction.Normalize()) return false;
	// 从 Pawn 内部发出，不能把枪口伸到墙后；忽略自己的受击体。
	const FVector Start = Pawn->GetActorLocation() + FVector(0.0, 0.0, 25.0);
	FVector End = Start + Direction * Range;
	FHitResult Hit;
	FCollisionQueryParams Query(SCENE_QUERY_STAT(ContraRifle), false, Pawn);
	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Query))
	{
		End = Hit.ImpactPoint;
		// 首轮关闭友伤；只伤害本玩法的敌人。
		if (AContraEnemy* Enemy = Cast<AContraEnemy>(Hit.GetActor()))
		{
			if (!Enemy->IsDead()) UExtHealthLibrary::ApplyDamage(Pawn->GetExtAbilitySystemComponent(), Enemy->GetAbilitySystemComponent(), Damage, Hit);
		}
	}
	ShowShot(Start, End);
	return true;
}

void UContraWeaponComponent::ShowShot_Implementation(FVector_NetQuantize Start, FVector_NetQuantize End)
{
	if (GetNetMode() != NM_DedicatedServer) DrawDebugLine(GetWorld(), Start, End, FColor::Yellow, false, 0.12f, 0, 3.0f);
}
