// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Editor/UnrealEdEngine.h"
#include "OmniEditorEngine.generated.h"

#define UE_API OMNIEDITOR_API

/**
 * 项目编辑器及 PIE 使用的引擎类。
 * 当前负责初始化时显示插件内容；独立游戏进程使用 OmniGameEngine。
 */
UCLASS(MinimalAPI)
class UOmniEditorEngine : public UUnrealEdEngine
{
	GENERATED_BODY()

public:
	UE_API virtual void Init(IEngineLoop* InEngineLoop) override;
};
#undef UE_API
