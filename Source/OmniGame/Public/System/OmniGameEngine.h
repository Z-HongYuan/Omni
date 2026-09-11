// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Engine/GameEngine.h"
#include "OmniGameEngine.generated.h"

#define UE_API OMNIGAME_API

/**
 * 项目独立运行时使用的引擎类。
 * 当前保留引擎初始化入口，后续在此添加项目级启动逻辑。
 * 编辑器及 PIE 使用 OmniEditorEngine。
 *
 * 对照 Lyra 5.8：原类同样仅保留构造与 Init 的父类调用，当前无额外功能缺口。
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
