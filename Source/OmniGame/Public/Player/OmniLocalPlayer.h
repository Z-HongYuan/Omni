// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "ExtLocalPlayer.h"
#include "OmniLocalPlayer.generated.h"

#define UE_API OMNIGAME_API

/**
 * 项目本地玩家，继承插件的玩家状态通知接口与视图开关。
 * 状态变更广播依赖 AExtPlayerController 协作，待项目控制器接入后验证。
 *
 * 对照 Lyra 5.8，后续按需评估迁入：
 * - 本地设置访问、共享设置的临时实例/异步加载，以及用户切换后的重新加载。
 * - 监听音频输出设备设置并切换设备。
 * - 跟随 PlayerController 的队伍状态，维护队伍变更委托及控制器切换时的绑定。
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
