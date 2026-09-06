// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Gameplay/ExperienceGameMode.h"
#include "OmniGameMode.generated.h"

#define UE_API OMNIGAME_API

/**
 * 项目使用的默认游戏模式类
 */
UCLASS(MinimalAPI)
class AOmniGameMode : public AExperienceGameMode
{
	GENERATED_BODY()

public:
	UE_API AOmniGameMode(const FObjectInitializer& ObjectInitializer);
};
#undef UE_API
