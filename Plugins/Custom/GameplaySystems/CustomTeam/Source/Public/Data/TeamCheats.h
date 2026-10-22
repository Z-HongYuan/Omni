// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "GameFramework/CheatManager.h"
#include "TeamCheats.generated.h"

#define UE_API CUSTOMTEAM_API

/** 提供切换玩家队伍和打印队伍 ID 的调试命令。 */
UCLASS(MinimalAPI)
class UTeamCheats : public UCheatManagerExtension
{
	GENERATED_BODY()

public:
	/** 按队伍 ID 顺序将当前玩家切换到下一个队伍。 */
	UFUNCTION(Exec, BlueprintAuthorityOnly)
	virtual void CycleTeam();

	/** 将当前玩家切换到已存在的指定队伍。 */
	UFUNCTION(Exec, BlueprintAuthorityOnly)
	virtual void SetTeam(int32 TeamID);

	/** 打印当前登记的全部队伍 ID。 */
	UFUNCTION(Exec)
	virtual void ListTeams();
};
#undef UE_API
