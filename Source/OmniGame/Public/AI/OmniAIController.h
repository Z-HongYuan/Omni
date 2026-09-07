// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Gameplay/ExperienceAIController.h"
#include "OmniAIController.generated.h"

#define UE_API OMNIGAME_API

/**
 * 项目使用的 AI 控制器基类
 */
UCLASS(MinimalAPI)
class AOmniAIController : public AExperienceAIController
{
	GENERATED_BODY()
};
#undef UE_API
