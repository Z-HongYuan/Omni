// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Components/ExpPlayerSpawningManagerComponent.h"
#include "ContraPlayerSpawningManagerComponent.generated.h"

/**
 * 魂斗罗玩家生成管理组件，由 Experience 的 Add Components Action 添加到服务端 GameState。
 * 当前沿用父类对 ExpPlayerStart 的缓存、占用检查和随机选点规则。
 */
UCLASS(MinimalAPI, meta = (DisplayName = "Contra Player Spawning Manager"))
class UContraPlayerSpawningManagerComponent : public UExpPlayerSpawningManagerComponent
{
	GENERATED_BODY()

public:
	OMNICONTRARUNTIME_API UContraPlayerSpawningManagerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};
