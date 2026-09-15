// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "ModularPawn.h"
#include "OmniPawn.generated.h"

#define UE_API OMNIGAME_API

/**
 * 项目空占位 Pawn，仅作为 GameMode 的默认兜底类型，不添加组件或玩法逻辑。
 * 正常体验通过 PawnData 选择 AOmniCharacter；模块化 Actor 生命周期沿用父类。
 * 对照 Lyra 5.8：体验初始化、ASC、输入与相机均不在本占位类中接入，按角色职责评估。
 */
UCLASS(MinimalAPI, Config = Game)
class AOmniPawn : public AModularPawn
{
	GENERATED_BODY()

public:
	UE_API AOmniPawn(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};

#undef UE_API
