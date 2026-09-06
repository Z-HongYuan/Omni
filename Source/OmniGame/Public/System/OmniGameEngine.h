// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Engine/GameEngine.h"
#include "OmniGameEngine.generated.h"

#define UE_API OMNIGAME_API

/**
 * 项目使用的引擎类
 * 暂时没有做任何事情
 */
UCLASS(MinimalAPI)
class UOmniGameEngine : public UGameEngine
{
	GENERATED_BODY()

public:
	// 暂时没有做任何事情
	UE_API virtual void Init(IEngineLoop* InEngineLoop) override;
};

#undef UE_API
