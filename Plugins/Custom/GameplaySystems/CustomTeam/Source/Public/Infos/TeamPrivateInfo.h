// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "TeamInfoBase.h"
#include "TeamPrivateInfo.generated.h"

#define UE_API CUSTOMTEAM_API

/*
 * 用于队伍中的私有信息
 */
UCLASS(MinimalAPI)
class ATeamPrivateInfo : public ATeamInfoBase
{
	GENERATED_BODY()

public:
	ATeamPrivateInfo(const FObjectInitializer& ObjectInitializer);

	/*
	 * 在队伍系统中使用的私有信息
	 * 保存当前队伍信息
	 */
};

#undef UE_API
