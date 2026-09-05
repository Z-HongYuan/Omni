// Copyright © 2026 张鸿源. All Rights Reserved.


#include "System/OmniGameViewportClient.h"

#include "OmniGame/OmniGameLogChannel.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(OmniGameViewportClient)

void UOmniGameViewportClient::Init(struct FWorldContext& WorldContext, UGameInstance* OwningGameInstance, bool bCreateNewAudioDevice)
{
	Super::Init(WorldContext, OwningGameInstance, bCreateNewAudioDevice);

	UE_LOG(LogOmniGame, Log, TEXT("OmniGameViewportClient Init"));
}
