// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "ExtPlayerController.h"
#include "OmniPlayerController.generated.h"

#define UE_API OMNIGAME_API

class UExtAbilitySystemComponent;

/**
 * 项目玩家控制器，由 GameMode 指定使用。
 * 插件父类已提供模块化组件接入，以及向 LocalPlayer 广播控制器、PlayerState 和 Pawn 的变化。
 * 解除控制前仅清空本 Pawn 的 ASC Avatar；完整清理及通知仍由 PawnExtension 负责。
 * PostProcessInput 统一处理 PlayerState ASC 的本帧能力输入。
 *
 * 对照 Lyra 5.8，后续按需评估迁入：
 * - PlayerState/HUD 的类型化访问。
 * - 跟随 PlayerState 的队伍状态与变更通知。
 * - 共享设置绑定、手柄力反馈策略，以及相机穿透时的目标隐藏。
 * - 自动奔跑、调试作弊入口和客户端回放录制；回放控制器可独立扩展。
 */
UCLASS(MinimalAPI, Config = Game)
class AOmniPlayerController : public AExtPlayerController
{
	GENERATED_BODY()

public:
	UE_API AOmniPlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UE_API UExtAbilitySystemComponent* GetASC() const;

	UE_API virtual void ReceivedPlayer() override;
	UE_API virtual void PostProcessInput(const float DeltaTime, const bool bGamePaused) override;

protected:
	UE_API virtual void OnUnPossess() override;
};

#undef UE_API
