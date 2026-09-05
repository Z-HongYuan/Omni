// Copyright © 2026 张鸿源. All Rights Reserved.


#include "System/OmniGameEngine.h"

#include "OmniGame/OmniGameLogChannel.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(OmniGameEngine)

void UOmniGameEngine::Init(IEngineLoop* InEngineLoop)
{
	Super::Init(InEngineLoop);

	UE_LOG(LogOmniGame, Log, TEXT("OmniGameEngine Init"));
}
