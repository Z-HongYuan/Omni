// Copyright © 2026 张鸿源. All Rights Reserved.


#include "ExtensionPlayerController.h"

#include "ExtensionLocalPlayer.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ExtensionPlayerController)

AExtensionPlayerController::AExtensionPlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void AExtensionPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	if (UExtensionLocalPlayer* LocalPlayer = Cast<UExtensionLocalPlayer>(Player))
	{
		LocalPlayer->OnPlayerControllerSet.Broadcast(LocalPlayer, this);

		if (PlayerState)
		{
			LocalPlayer->OnPlayerStateSet.Broadcast(LocalPlayer, PlayerState);
		}
	}
}

void AExtensionPlayerController::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);

	if (UExtensionLocalPlayer* LocalPlayer = Cast<UExtensionLocalPlayer>(Player))
	{
		LocalPlayer->OnPlayerPawnSet.Broadcast(LocalPlayer, InPawn);
	}
}

void AExtensionPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	if (PlayerState)
	{
		if (UExtensionLocalPlayer* LocalPlayer = Cast<UExtensionLocalPlayer>(Player))
		{
			LocalPlayer->OnPlayerStateSet.Broadcast(LocalPlayer, PlayerState);
		}
	}
}

void AExtensionPlayerController::OnPossess(class APawn* APawn)
{
	Super::OnPossess(APawn);

	if (UExtensionLocalPlayer* LocalPlayer = Cast<UExtensionLocalPlayer>(Player))
	{
		LocalPlayer->OnPlayerPawnSet.Broadcast(LocalPlayer, APawn);
	}
}

void AExtensionPlayerController::OnUnPossess()
{
	Super::OnUnPossess();

	if (UExtensionLocalPlayer* LocalPlayer = Cast<UExtensionLocalPlayer>(Player))
	{
		LocalPlayer->OnPlayerPawnSet.Broadcast(LocalPlayer, nullptr);
	}
}
