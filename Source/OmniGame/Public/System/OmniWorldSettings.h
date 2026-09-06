// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Helper/ExperienceWorldSettings.h"

#include "OmniWorldSettings.generated.h"

#define UE_API OMNIGAME_API

/**
 * 项目使用的世界设置类
 * 暂时没有做任何事情
 */
UCLASS(MinimalAPI)
class AOmniWorldSettings : public AExperienceWorldSettings
{
	GENERATED_BODY()
};
#undef UE_API
