// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Gameplay/ExpPlayerStart.h"

#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "TimerManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ExpPlayerStart)

AExpPlayerStart::AExpPlayerStart(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

EExpPlayerStartLocationOccupancy AExpPlayerStart::GetLocationOccupancy(AController* const ControllerPawnToFit) const
{
	UWorld* World = GetWorld();
	if (HasAuthority() && World)
	{
		if (AGameModeBase* AuthGameMode = World->GetAuthGameMode())
		{
			// 按控制器实际使用的 Pawn 类型检测空间，避免不同角色体积不一致。
			TSubclassOf<APawn> PawnClass = AuthGameMode->GetDefaultPawnClassForController(ControllerPawnToFit);
			const APawn* const PawnToFit = PawnClass ? GetDefault<APawn>(PawnClass) : nullptr;

			FVector ActorLocation = GetActorLocation();
			const FRotator ActorRotation = GetActorRotation();

			// 无碰撞阻挡时可直接生成。
			if (!World->EncroachingBlockingGeometry(PawnToFit, ActorLocation, ActorRotation, nullptr))
			{
				return EExpPlayerStartLocationOccupancy::Empty;
			}

			if (World->FindTeleportSpot(PawnToFit, ActorLocation, ActorRotation))
			{
				return EExpPlayerStartLocationOccupancy::Partial;
			}
		}
	}

	return EExpPlayerStartLocationOccupancy::Full;
}

bool AExpPlayerStart::IsClaimed() const
{
	return ClaimingController != nullptr;
}

bool AExpPlayerStart::TryClaim(AController* OccupyingController)
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

void AExpPlayerStart::CheckUnclaimed()
{
	// 控制器已有 Pawn 且出生点无阻挡时，释放声明占用。
	if (ClaimingController != nullptr && ClaimingController->GetPawn() != nullptr && GetLocationOccupancy(ClaimingController) == EExpPlayerStartLocationOccupancy::Empty)
	{
		ClaimingController = nullptr;
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(ExpirationTimerHandle);
		}
	}
}
