// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "ModularPawn.h"
#include "OmniPawn.generated.h"

#define UE_API OMNIGAME_API

/** GameMode 的空占位 Pawn，未指定角色类型时作为兜底，不额外添加组件或玩法逻辑。 */
UCLASS(MinimalAPI, Config = Game)
class AOmniPawn : public AModularPawn
{
	GENERATED_BODY()

public:
	UE_API AOmniPawn(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};

#undef UE_API
