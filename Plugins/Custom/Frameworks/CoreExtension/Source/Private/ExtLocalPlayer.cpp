// Copyright © 2026 张鸿源. All Rights Reserved.


#include "ExtLocalPlayer.h"

#include "Engine/GameInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ExtLocalPlayer)

UExtLocalPlayer::UExtLocalPlayer(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

FDelegateHandle UExtLocalPlayer::CallOrRegister_OnPlayerControllerSet(const FPlayerControllerSetDelegate::FDelegate& Delegate)
{
	APlayerController* PC = GetPlayerController(GetWorld());

	if (PC) Delegate.Execute(this, PC);

	return OnPlayerControllerSet.Add(Delegate);
}

FDelegateHandle UExtLocalPlayer::CallOrRegister_OnPlayerStateSet(const FPlayerStateSetDelegate::FDelegate& Delegate)
{
	APlayerController* PC = GetPlayerController(GetWorld());
	APlayerState* PlayerState = PC ? PC->PlayerState : nullptr;

	if (PlayerState) Delegate.Execute(this, PlayerState);

	return OnPlayerStateSet.Add(Delegate);
}

FDelegateHandle UExtLocalPlayer::CallOrRegister_OnPlayerPawnSet(const FPlayerPawnSetDelegate::FDelegate& Delegate)
{
	APlayerController* PC = GetPlayerController(GetWorld());
	APawn* Pawn = PC ? PC->GetPawn() : nullptr;

	if (Pawn) Delegate.Execute(this, Pawn);

	return OnPlayerPawnSet.Add(Delegate);
}

bool UExtLocalPlayer::GetProjectionData(FViewport* Viewport, FSceneViewProjectionData& ProjectionData, int32 StereoViewIndex) const
{
	if (!bIsPlayerViewEnabled) { return false; }
	return Super::GetProjectionData(Viewport, ProjectionData, StereoViewIndex);
}
