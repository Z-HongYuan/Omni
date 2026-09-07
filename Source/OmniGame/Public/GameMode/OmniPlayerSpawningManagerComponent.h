// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Components/ExperiencePlayerSpawningManagerComponent.h"
#include "OmniPlayerSpawningManagerComponent.generated.h"

#define UE_API OMNIGAME_API

/**
 * 项目使用的玩家生成管理组件
 * 需要手动使用Action加载到GameState中
 * 目前没有做任何事情
 * 需要蓝图继承并且修改配置
 */
UCLASS(MinimalAPI)
class UOmniPlayerSpawningManagerComponent : public UExperiencePlayerSpawningManagerComponent
{
	GENERATED_BODY()

public:
};
#undef UE_API
