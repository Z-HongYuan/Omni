// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Engine/GameEngine.h"
#include "OmniGameEngine.generated.h"

#define UE_API OMNIGAME_API

/**
 * 项目独立游戏进程使用的引擎类，初始化沿用父类流程。
 * 编辑器及 PIE 使用 OmniEditorEngine。
 */
UCLASS(MinimalAPI)
class UOmniGameEngine : public UGameEngine
{
	GENERATED_BODY()

public:
	UE_API UOmniGameEngine(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UE_API virtual void Init(IEngineLoop* InEngineLoop) override;
};

#undef UE_API
