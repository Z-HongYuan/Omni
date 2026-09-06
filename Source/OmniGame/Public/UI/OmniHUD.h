// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "ModularHUD.h"
#include "OmniHUD.generated.h"

#define UE_API OMNIGAME_API

/**
 * 项目使用的默认HUD类
 */
UCLASS(MinimalAPI)
class AOmniHUD : public AModularHUD
{
	GENERATED_BODY()
};
#undef UE_API
