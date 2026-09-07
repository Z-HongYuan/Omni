// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Components/ExperienceBotCreationComponent.h"
#include "OmniBotCreationComponent.generated.h"

#define UE_API OMNIGAME_API

/**
 * 项目使用的机器人创建组件
 * 需要手动将此组件添加到GameState中
 * 目前没有做任何事情
 * 需要蓝图继承并且修改配置
 */
UCLASS(MinimalAPI)
class UOmniBotCreationComponent : public UExperienceBotCreationComponent
{
	GENERATED_BODY()

public:
};
#undef UE_API
