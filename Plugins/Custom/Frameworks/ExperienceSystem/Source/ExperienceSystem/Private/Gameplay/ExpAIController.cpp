// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Gameplay/ExpAIController.h"

#include "Engine/World.h"
#include "Gameplay/ExpGameMode.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ExpAIController)

AExpAIController::AExpAIController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Bot 也需要 PlayerState，以提供 PawnData 和 ASC。
	bWantsPlayerState = true;

	// 解除控制时保留 AI 逻辑，避免重生流程中断行为树。
	bStopAILogicOnUnposses = false;
}

void AExpAIController::ServerRestartController()
{
	// 仅服务器执行重生。
	if (GetNetMode() == NM_Client) return;

	// 只处理 Inactive 或 Spectating 状态的控制器。
	if (IsInState(NAME_Inactive) || IsInState(NAME_Spectating))
	{
		AExpGameMode* const GameMode = GetWorld()->GetAuthGameMode<AExpGameMode>();

		// 通过 GameMode 统一检查 Bot 是否允许重生。
		if ((GameMode == nullptr) || !GameMode->ControllerCanRestart(this)) return;

		// 先解除旧 Pawn 的控制关系。
		if (GetPawn() != nullptr)
			UnPossess();

		// 重生前重置输入忽略标记。
		ResetIgnoreInputFlags();

		GameMode->RestartPlayer(this);
	}
}
