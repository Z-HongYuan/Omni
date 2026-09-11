// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Gameplay/ExpAIController.h"

#include "Engine/World.h"
#include "Gameplay/ExpGameMode.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ExpAIController)

AExpAIController::AExpAIController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Bot 同样需要 PlayerState，因为 PawnData 与 ASC 都挂在 PS 上
	bWantsPlayerState = true;

	// 取消 Possess 时不停止 AI 逻辑，避免重生流程中行为树被打断
	bStopAILogicOnUnposses = false;
}

void AExpAIController::ServerRestartController()
{
	// 仅服务端有权限重生
	if (GetNetMode() == NM_Client) return;

	// 只有处于 Inactive 或 Spectating 状态的控制器才需要重生
	if (IsInState(NAME_Inactive) || IsInState(NAME_Spectating))
	{
		AExpGameMode* const GameMode = GetWorld()->GetAuthGameMode<AExpGameMode>();

		// 询问 GameMode 是否允许重生,注意这里是 AIController
		if ((GameMode == nullptr) || !GameMode->ControllerCanRestart(this)) return;

		// 如果还挂着一个 Pawn，先脱离
		if (GetPawn() != nullptr)
			UnPossess();

		// 重新启用输入，与 ClientRestart 中的处理保持一致
		ResetIgnoreInputFlags();

		GameMode->RestartPlayer(this);
	}
}
