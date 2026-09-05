// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Abilities/GameplayAbilityTargetActor_Interact.h"
#include "GameFramework/LightWeightInstanceSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameplayAbilityTargetActor_Interact)

AGameplayAbilityTargetActor_Interact::AGameplayAbilityTargetActor_Interact()
{
	PrimaryActorTick.bCanEverTick = false;
}

FHitResult AGameplayAbilityTargetActor_Interact::PerformTrace(AActor* InSourceActor)
{
	bool bTraceComplex = false;

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(InSourceActor);

	FCollisionQueryParams Params(SCENE_QUERY_STAT(AGameplayAbilityTargetActor_SingleLineTrace), bTraceComplex);
	Params.bReturnPhysicalMaterial = true;
	Params.AddIgnoredActors(ActorsToIgnore);

	FVector TraceStart = StartLocation.GetTargetingTransform().GetLocation(); // InSourceActor->GetActorLocation();
	FVector TraceEnd;
	AimWithPlayerController(InSourceActor, Params, TraceStart, TraceEnd); // 仅在服务器和启动客户端生效

	// ------------------------------------------------------

	FHitResult ReturnHitResult;
	LineTraceWithFilter(ReturnHitResult, InSourceActor->GetWorld(), Filter, TraceStart, TraceEnd, TraceProfile.Name, Params);
	// 如果没碰到任何东西，默认切换到追踪线末端。
	if (!ReturnHitResult.bBlockingHit)
	{
		ReturnHitResult.Location = TraceEnd;
	}
	if (AGameplayAbilityWorldReticle* LocalReticleActor = ReticleActor.Get())
	{
		const bool bHitActor = (ReturnHitResult.bBlockingHit && (ReturnHitResult.HitObjectHandle.IsValid()));
		const FVector ReticleLocation = (bHitActor && LocalReticleActor->bSnapToTargetedActor) ? FLightWeightInstanceSubsystem::Get().GetLocation(ReturnHitResult.HitObjectHandle) : ReturnHitResult.Location;

		LocalReticleActor->SetActorLocation(ReticleLocation);
		LocalReticleActor->SetIsTargetAnActor(bHitActor);
	}

#if ENABLE_DRAW_DEBUG
	if (bDebug)
	{
		DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Green);
		DrawDebugSphere(GetWorld(), TraceEnd, 100.0f, 16, FColor::Green);
	}
#endif
	return ReturnHitResult;
}
