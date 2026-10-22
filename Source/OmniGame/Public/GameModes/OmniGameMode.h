// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Gameplay/ExpGameMode.h"
#include "OmniGameMode.generated.h"

#define UE_API OMNIGAME_API

/**
 * 项目游戏模式，指定项目 GameState、PlayerController 和 PlayerState，并提供默认体验兜底。
 * 体验加载、PawnData 注入及玩家生成由 ExperienceSystem 父类管理。
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
