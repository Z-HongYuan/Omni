// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Player/OmniLocalPlayer.h"

#include "OmniGame/OmniGameLogChannel.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(OmniLocalPlayer)

UOmniLocalPlayer::UOmniLocalPlayer(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UOmniLocalPlayer::PlayerAdded(UGameViewportClient* InViewportClient, int32 InControllerID)
{
	Super::PlayerAdded(InViewportClient, InControllerID);

	UE_LOG(LogOmniGame, Log, TEXT("OmniLocalPlayer PlayerAdded: %s (%s)"), *GetPathName(), *GetClass()->GetPathName());
}

void UOmniLocalPlayer::PlayerAdded(UGameViewportClient* InViewportClient, FPlatformUserId InUserId)
{
	Super::PlayerAdded(InViewportClient, InUserId);

	UE_LOG(LogOmniGame, Log, TEXT("OmniLocalPlayer PlayerAdded: %s (%s)"), *GetPathName(), *GetClass()->GetPathName());
}

void UOmniLocalPlayer::PlayerRemoved()
{
	UE_LOG(LogOmniGame, Log, TEXT("OmniLocalPlayer PlayerRemoved: %s"), *GetPathName());

	Super::PlayerRemoved();
}
