// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "ExtensionGameInstance.h"

#include "OmniGameInstance.generated.h"

#define UE_API OMNIGAME_API

/**
 * 项目使用的游戏实例
 * 暂时没有做任何实现
 */
UCLASS(MinimalAPI)
class UOmniGameInstance : public UExtensionGameInstance
{
	GENERATED_BODY()

public:
};
#undef UE_API
