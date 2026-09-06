// Copyright © 2026 张鸿源. All Rights Reserved.


#include "GameMode/OmniGameMode.h"

#include "GameMode/OmniGameState.h"
#include "Player/OmniCharacterBase.h"
#include "Player/OmniPlayerController.h"
#include "Player/OmniPlayerState.h"
#include "Player/OmniReplayPlayerController.h"
#include "System/OmniGameSession.h"
#include "UI/OmniHUD.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(OmniGameMode)

AOmniGameMode::AOmniGameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	GameStateClass = AOmniGameState::StaticClass();
	GameSessionClass = AOmniGameSession::StaticClass();
	PlayerControllerClass = AOmniPlayerController::StaticClass();
	ReplaySpectatorPlayerControllerClass = AOmniReplayPlayerController::StaticClass();
	PlayerStateClass = AOmniPlayerState::StaticClass();
	DefaultPawnClass = AOmniCharacterBase::StaticClass();
	HUDClass = AOmniHUD::StaticClass();
}
