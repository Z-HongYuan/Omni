// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "ExtensionLocalPlayer.h"
#include "OmniLocalPlayer.generated.h"

#define UE_API OMNIGAME_API

/**
 * 项目使用的本地玩家类
 */
UCLASS(MinimalAPI)
class UOmniLocalPlayer : public UExtensionLocalPlayer
{
	GENERATED_BODY()
};
#undef UE_API
