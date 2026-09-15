// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Gameplay/ExpPlayerState.h"
#include "OmniPlayerState.generated.h"

#define UE_API OMNIGAME_API

/**
 * 项目玩家状态，沿用父类的玩家 ASC、PawnData 复制、体验就绪后的能力授予与初始化通知。
 * Pawn 的 ASC Avatar 由项目初始化组件协同插件 Pawn 扩展组件绑定，此处保留玩家 ASC 的所有权。
 *
 * 对照 Lyra 5.8，后续按需评估迁入：
 * - 生命/战斗 AttributeSet，以及项目 PlayerController 的类型化访问。
 * - 队伍与小队状态、变更通知，以及用于统计的 GameplayTag 计数。
 * - 玩家连接/观战状态，断线与恢复策略，以及切图时的状态复制。
 * - 面向单个玩家的玩法消息，以及观战所需的视角旋转同步。
 */
UCLASS(MinimalAPI, Config = Game)
class AOmniPlayerState : public AExpPlayerState
{
	GENERATED_BODY()

public:
	UE_API AOmniPlayerState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	UE_API virtual void OnExperienceLoaded(const UExpDefinition* CurrentExperience) override;
};

#undef UE_API
