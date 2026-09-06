// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Engine/AssetManager.h"
#include "OmniAssetManager.generated.h"

#define UE_API OMNIGAME_API

/**
 * 项目使用的默认资产管理器
 */
UCLASS(MinimalAPI)
class UOmniAssetManager : public UAssetManager
{
	GENERATED_BODY()
};
#undef UE_API
