// Copyright © 2026 张鸿源. All Rights Reserved.


#include "GameModes/OmniGameState.h"

#include "OmniGame/OmniGameLogChannel.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(OmniGameState)

AOmniGameState::AOmniGameState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void AOmniGameState::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	UE_LOG(LogOmniGame, Log, TEXT("OmniGameState PostInitializeComponents: %s (%s)"), *GetPathName(), *GetClass()->GetPathName());
}
