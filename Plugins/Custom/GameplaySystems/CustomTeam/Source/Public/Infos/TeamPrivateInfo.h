// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "TeamInfoBase.h"
#include "TeamPrivateInfo.generated.h"

#define UE_API CUSTOMTEAM_API

/** 队伍私有信息的扩展类型；具体字段与可见性策略由项目补充。 */
UCLASS(MinimalAPI)
class ATeamPrivateInfo : public ATeamInfoBase
{
	GENERATED_BODY()

public:
	ATeamPrivateInfo(const FObjectInitializer& ObjectInitializer);

	// 在项目子类中补充所需的队伍信息。
};

#undef UE_API
