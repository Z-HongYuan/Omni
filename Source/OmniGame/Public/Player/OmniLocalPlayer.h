// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "ExtLocalPlayer.h"
#include "OmniLocalPlayer.generated.h"

#define UE_API OMNIGAME_API

/**
 * 项目本地玩家，继承插件的玩家状态通知接口与视图开关。
 * 控制器、PlayerState 和 Pawn 的状态通知由 AExtPlayerController 协同驱动。
 */
UCLASS(MinimalAPI, Transient, Config = Game)
class UOmniLocalPlayer : public UExtLocalPlayer
{
	GENERATED_BODY()

public:
	UE_API UOmniLocalPlayer(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UE_API virtual void PlayerAdded(UGameViewportClient* InViewportClient, int32 InControllerID) override;
	UE_API virtual void PlayerAdded(UGameViewportClient* InViewportClient, FPlatformUserId InUserId) override;
	UE_API virtual void PlayerRemoved() override;
};

#undef UE_API
