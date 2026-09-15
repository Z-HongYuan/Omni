// Copyright © 2026 张鸿源. All Rights Reserved.


#include "GameModes/OmniGameMode.h"

#include "Character/OmniPawn.h"
#include "GameModes/OmniGameState.h"
#include "OmniGame/OmniGameLogChannel.h"
#include "Player/OmniPlayerController.h"
#include "Player/OmniPlayerState.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(OmniGameMode)

AOmniGameMode::AOmniGameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	GameStateClass = AOmniGameState::StaticClass();
	PlayerControllerClass = AOmniPlayerController::StaticClass();
	PlayerStateClass = AOmniPlayerState::StaticClass();
	// 体验 PawnData 优先选择实际角色；未指定角色时才使用空占位 Pawn。
	DefaultPawnClass = AOmniPawn::StaticClass();
}

void AOmniGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	UE_LOG(LogOmniGame, Log, TEXT("OmniGameMode InitGame: %s (%s), Map: %s"), *GetPathName(), *GetClass()->GetPathName(), *MapName);
}

FPrimaryAssetId AOmniGameMode::GetFallbackExperienceId() const
{
	// URL、开发者设置和世界设置都未指定体验时，使用项目的最小默认定义。
	return FPrimaryAssetId(FPrimaryAssetType(UExpDefinition::StaticClass()->GetFName()), TEXT("DA_DefaultExperience"));
}
