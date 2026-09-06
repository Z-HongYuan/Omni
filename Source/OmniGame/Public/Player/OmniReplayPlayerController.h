// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Player/OmniPlayerController.h"
#include "OmniReplayPlayerController.generated.h"

#define UE_API OMNIGAME_API

/**
 * 项目使用的回放观战玩家控制器类
 */
UCLASS(MinimalAPI)
class AOmniReplayPlayerController : public AOmniPlayerController
{
	GENERATED_BODY()

public:
	UE_API AOmniReplayPlayerController(const FObjectInitializer& ObjectInitializer);
};
#undef UE_API