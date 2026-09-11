// Copyright © 2026 张鸿源. All Rights Reserved.


#include "ExtPlayerController.h"

#include "ExtLocalPlayer.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ExtPlayerController)

AExtPlayerController::AExtPlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void AExtPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	if (UExtLocalPlayer* LocalPlayer = Cast<UExtLocalPlayer>(Player))
	{
		LocalPlayer->OnPlayerControllerSet.Broadcast(LocalPlayer, this);

		if (PlayerState)
		{
			LocalPlayer->OnPlayerStateSet.Broadcast(LocalPlayer, PlayerState);
		}
	}
}

void AExtPlayerController::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);

	if (UExtLocalPlayer* LocalPlayer = Cast<UExtLocalPlayer>(Player))
	{
		LocalPlayer->OnPlayerPawnSet.Broadcast(LocalPlayer, InPawn);
	}
}

void AExtPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	if (PlayerState)
	{
		if (UExtLocalPlayer* LocalPlayer = Cast<UExtLocalPlayer>(Player))
		{
			LocalPlayer->OnPlayerStateSet.Broadcast(LocalPlayer, PlayerState);
		}
	}
}

void AExtPlayerController::OnPossess(class APawn* APawn)
{
	Super::OnPossess(APawn);

	if (UExtLocalPlayer* LocalPlayer = Cast<UExtLocalPlayer>(Player))
	{
		LocalPlayer->OnPlayerPawnSet.Broadcast(LocalPlayer, APawn);
	}
}

void AExtPlayerController::OnUnPossess()
{
	Super::OnUnPossess();

	if (UExtLocalPlayer* LocalPlayer = Cast<UExtLocalPlayer>(Player))
	{
		LocalPlayer->OnPlayerPawnSet.Broadcast(LocalPlayer, nullptr);
	}
}
