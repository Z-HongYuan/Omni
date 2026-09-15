// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Gameplay/ExpGameMode.h"
#include "OmniGameMode.generated.h"

#define UE_API OMNIGAME_API

/**
 * 项目游戏模式，继承 ExperienceSystem 的体验驱动流程，并指定项目 GameState、PlayerController 和 PlayerState。
 * 项目 GS/PS 沿用插件父类能力，玩家生成需等待有效 Experience 加载完成。
 * 插件已负责体验选择、PawnData 注入、出生点与重生扩展，以及玩家初始化通知。
 *
 * 对照 Lyra 5.8，后续按需评估接入：
 * - 当前默认体验通过 PawnData 选择 Character，空 Pawn 仅作兜底；GF、输入和能力配置后续按玩法补充。
 * - 专用服务器登录与建局策略：通过父类对应钩子补充。
 * - HUD、GameSession 和回放控制器的默认类型组合。
 */
UCLASS(MinimalAPI, Config = Game)
class AOmniGameMode : public AExpGameMode
{
	GENERATED_BODY()

public:
	UE_API AOmniGameMode(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UE_API virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

protected:
	UE_API virtual FPrimaryAssetId GetFallbackExperienceId() const override;
};

#undef UE_API
