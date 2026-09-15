// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Gameplay/ExpGameState.h"
#include "OmniGameState.generated.h"

#define UE_API OMNIGAME_API

/**
 * 项目对局状态，沿用父类的 ExperienceManager 和模块化组件接入。
 * 体验定义及加载状态由插件管理，项目层不重复创建管理器。
 *
 * 对照 Lyra 5.8，后续按需评估迁入：
 * - 用于对局级能力与 GameplayCue 的独立 ASC；区别于 PlayerState 上的玩家 ASC。
 * - 向客户端广播玩法消息，以及服务器 FPS 的采集与同步。
 * - 回放录制玩家的记录、同步与变更通知。
 */
UCLASS(MinimalAPI, Config = Game)
class AOmniGameState : public AExpGameState
{
	GENERATED_BODY()

public:
	UE_API AOmniGameState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UE_API virtual void PostInitializeComponents() override;
};

#undef UE_API
