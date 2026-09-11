// Copyright © 2026 张鸿源. All Rights Reserved.


#include "ExtensionLocalPlayer.h"

#include "Engine/GameInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ExtensionLocalPlayer)

UExtensionLocalPlayer::UExtensionLocalPlayer(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

FDelegateHandle UExtensionLocalPlayer::CallOrRegister_OnPlayerControllerSet(const FPlayerControllerSetDelegate::FDelegate& Delegate)
{
	APlayerController* PC = GetPlayerController(GetWorld());

	if (PC) Delegate.Execute(this, PC);

	return OnPlayerControllerSet.Add(Delegate);
}

FDelegateHandle UExtensionLocalPlayer::CallOrRegister_OnPlayerStateSet(const FPlayerStateSetDelegate::FDelegate& Delegate)
{
	APlayerController* PC = GetPlayerController(GetWorld());
	APlayerState* PlayerState = PC ? PC->PlayerState : nullptr;

	if (PlayerState) Delegate.Execute(this, PlayerState);

	return OnPlayerStateSet.Add(Delegate);
}

FDelegateHandle UExtensionLocalPlayer::CallOrRegister_OnPlayerPawnSet(const FPlayerPawnSetDelegate::FDelegate& Delegate)
{
	APlayerController* PC = GetPlayerController(GetWorld());
	APawn* Pawn = PC ? PC->GetPawn() : nullptr;

	if (Pawn) Delegate.Execute(this, Pawn);

	return OnPlayerPawnSet.Add(Delegate);
}

bool UExtensionLocalPlayer::GetProjectionData(FViewport* Viewport, FSceneViewProjectionData& ProjectionData, int32 StereoViewIndex) const
{
	if (!bIsPlayerViewEnabled) { return false; }
	return Super::GetProjectionData(Viewport, ProjectionData, StereoViewIndex);
}
