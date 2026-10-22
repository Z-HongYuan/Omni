// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "GameFramework/CheatManager.h"
#include "ContraCheats.generated.h"

#define UE_API OMNICONTRARUNTIME_API

/** Contra 玩法的作弊扩展。 */
UCLASS(MinimalAPI, NotBlueprintable)
class UContraCheats : public UCheatManagerExtension
{
	GENERATED_BODY()

public:
	// 杀死当前控制的角色，沿用 Health 的自毁伤害与死亡流程。
	UFUNCTION(Exec, BlueprintAuthorityOnly)
	UE_API void KillPlayer();
};

#undef UE_API
