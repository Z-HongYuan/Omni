// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Editor/UnrealEdEngine.h"
#include "OmniEditorEngine.generated.h"

#define UE_API OMNIEDITOR_API

/**
 * 项目编辑器使用的引擎类
 */
UCLASS(MinimalAPI)
class UOmniEditorEngine : public UUnrealEdEngine
{
	GENERATED_BODY()

public:
	UE_API virtual void Init(IEngineLoop* InEngineLoop) override;
};
#undef UE_API
