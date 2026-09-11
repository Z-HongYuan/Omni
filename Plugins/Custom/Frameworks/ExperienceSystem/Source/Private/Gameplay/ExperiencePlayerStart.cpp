// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Gameplay/ExperiencePlayerStart.h"

#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "TimerManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ExperiencePlayerStart)

AExperiencePlayerStart::AExperiencePlayerStart(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

EExperiencePlayerStartLocationOccupancy AExperiencePlayerStart::GetLocationOccupancy(AController* const ControllerPawnToFit) const
{
	UWorld* World = GetWorld();
	if (HasAuthority() && World)
	{
		if (AGameModeBase* AuthGameMode = World->GetAuthGameMode())
		{
			// 用这个控制器真正会生成的 Pawn 类来做体积检测
			TSubclassOf<APawn> PawnClass = AuthGameMode->GetDefaultPawnClassForController(ControllerPawnToFit);
			const APawn* const PawnToFit = PawnClass ? GetDefault<APawn>(PawnClass) : nullptr;

			FVector ActorLocation = GetActorLocation();
			const FRotator ActorRotation = GetActorRotation();

			// 没有遮挡就是空闲
			if (!World->EncroachingBlockingGeometry(PawnToFit, ActorLocation, ActorRotation, nullptr))
			{
				return EExperiencePlayerStartLocationOccupancy::Empty;
			}

			if (World->FindTeleportSpot(PawnToFit, ActorLocation, ActorRotation))
			{
				return EExperiencePlayerStartLocationOccupancy::Partial;
			}
		}
	}

	return EExperiencePlayerStartLocationOccupancy::Full;
}

bool AExperiencePlayerStart::IsClaimed() const
{
	return ClaimingController != nullptr;
}

bool AExperiencePlayerStart::TryClaim(AController* OccupyingController)
{
	if (OccupyingController != nullptr && !IsClaimed())
	{
		ClaimingController = OccupyingController;
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(ExpirationTimerHandle, FTimerDelegate::CreateUObject(this, &ThisClass::CheckUnclaimed), ExpirationCheckInterval, true);
		}
		return true;
	}
	return false;
}

void AExperiencePlayerStart::CheckUnclaimed()
{
	// 占用者已经生成了 Pawn 并且离开了此位置，就可以释放占用
	if (ClaimingController != nullptr && ClaimingController->GetPawn() != nullptr && GetLocationOccupancy(ClaimingController) == EExperiencePlayerStartLocationOccupancy::Empty)
	{
		ClaimingController = nullptr;
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(ExpirationTimerHandle);
		}
	}
}
