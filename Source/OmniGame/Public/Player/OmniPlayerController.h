// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "ExtensionPlayerController.h"
#include "OmniPlayerController.generated.h"

#define UE_API OMNIGAME_API

/**
 * 项目使用的默认玩家控制器类
 */
UCLASS(MinimalAPI)
class AOmniPlayerController : public AExtensionPlayerController
{
	GENERATED_BODY()
};
#undef UE_API
