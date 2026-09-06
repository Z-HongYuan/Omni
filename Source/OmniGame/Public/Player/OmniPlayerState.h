// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Gameplay/ExperiencePlayerState.h"
#include "OmniPlayerState.generated.h"

#define UE_API OMNIGAME_API

/**
 * 项目使用的默认玩家状态类
 */
UCLASS(MinimalAPI)
class AOmniPlayerState : public AExperiencePlayerState
{
	GENERATED_BODY()
};
#undef UE_API
