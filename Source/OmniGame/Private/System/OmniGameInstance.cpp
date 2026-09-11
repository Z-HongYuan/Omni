// Copyright © 2026 张鸿源. All Rights Reserved.


#include "System/OmniGameInstance.h"

#include "OmniGame/OmniGameLogChannel.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(OmniGameInstance)

UOmniGameInstance::UOmniGameInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UOmniGameInstance::Init()
{
	Super::Init();

	UE_LOG(LogOmniGame, Log, TEXT("OmniGameInstance Init: %s (%s)"), *GetPathName(), *GetClass()->GetPathName());
}

void UOmniGameInstance::Shutdown()
{
	UE_LOG(LogOmniGame, Log, TEXT("OmniGameInstance Shutdown: %s"), *GetPathName());

	Super::Shutdown();
}
