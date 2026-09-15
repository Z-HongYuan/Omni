// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Player/OmniPlayerController.h"

#include "Gameplay/ExpPlayerState.h"
#include "OmniGame/OmniGameLogChannel.h"
#include "System/ExtAbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(OmniPlayerController)

AOmniPlayerController::AOmniPlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UExtAbilitySystemComponent* AOmniPlayerController::GetASC() const
{
	const AExpPlayerState* PS = GetPlayerState<AExpPlayerState>();
	return IsValid(PS) ? PS->GetExtAbilitySystemComponent() : nullptr;
}

void AOmniPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	UE_LOG(LogOmniGame, Log, TEXT("OmniPlayerController ReceivedPlayer: %s (%s)"), *GetPathName(), *GetClass()->GetPathName());
}

void AOmniPlayerController::PostProcessInput(const float DeltaTime, const bool bGamePaused)
{
	// 与 Lyra 一致：输入收集完成后，先处理 PlayerState ASC 的能力输入，再调用父类。
	if (UExtAbilitySystemComponent* ASC = GetASC())
	{
		ASC->ProcessAbilityInput(DeltaTime, bGamePaused);
	}

	Super::PostProcessInput(DeltaTime, bGamePaused);
}

void AOmniPlayerController::OnUnPossess()
{
	// 与 Lyra PC 一致：在父类解除控制前清空 Avatar，不取消能力或广播完整注销。
	if (APawn* PawnBeingUnpossessed = GetPawn())
	{
		if (UExtAbilitySystemComponent* ASC = GetASC())
		{
			if (ASC->GetAvatarActor() == PawnBeingUnpossessed)
			{
				ASC->SetAvatarActor(nullptr);
			}
		}
	}

	Super::OnUnPossess();
}
