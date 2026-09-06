// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "GameFramework/GameSession.h"
#include "OmniGameSession.generated.h"

#define UE_API OMNIGAME_API

/**
 * 项目使用的游戏会话类
 * 暂时没有做任何事情
 * 以后将会在类中添加更多的设置项,以满足项目的需求
 */
UCLASS(MinimalAPI, Config = Game)
class AOmniGameSession : public AGameSession
{
	GENERATED_BODY()

public:
	UE_API AOmniGameSession(const FObjectInitializer& ObjectInitializer);
};
#undef UE_API