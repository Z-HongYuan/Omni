// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "ModularHUD.h"
#include "OmniHUD.generated.h"

#define UE_API OMNIGAME_API

/**
 * 项目使用的默认HUD类
 */
UCLASS(MinimalAPI)
class AOmniHUD : public AModularHUD
{
	GENERATED_BODY()

public:
	UE_API AOmniHUD(const FObjectInitializer& ObjectInitializer);
};
#undef UE_API

// 通常不需要扩展或修改此类
// 在您Experience中使用"添加小部件/Add Widget"操作，向其中添加HUD布局和小部件
//
// 此类主要用于调试渲染
