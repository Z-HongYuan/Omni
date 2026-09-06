// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Gameplay/ExperienceGameState.h"
#include "OmniGameState.generated.h"

#define UE_API OMNIGAME_API

/**
 * 项目使用的默认游戏状态类
 */
UCLASS(MinimalAPI)
class AOmniGameState : public AExperienceGameState
{
	GENERATED_BODY()
};
#undef UE_API
