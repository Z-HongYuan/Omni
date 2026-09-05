// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "GameFramework/CheatManager.h"
#include "TeamCheats.generated.h"

#define UE_API CUSTOMTEAM_API

/**
 * 
 */
UCLASS(MinimalAPI)
class UTeamCheats : public UCheatManagerExtension
{
	GENERATED_BODY()

public:
	//将此玩家移动到下一个可用的团队
	UFUNCTION(Exec, BlueprintAuthorityOnly)
	virtual void CycleTeam();

	// 将该球员移动到指定队伍
	UFUNCTION(Exec, BlueprintAuthorityOnly)
	virtual void SetTeam(int32 TeamID);

	// 打印所有队伍名单
	UFUNCTION(Exec)
	virtual void ListTeams();
};
#undef UE_API
