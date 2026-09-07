// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Player/OmniPlayerController.h"

#include "Gameplay/ExperiencePlayerState.h"
#include "System/CustomAbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(OmniPlayerController)

AOmniPlayerController::AOmniPlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UCustomAbilitySystemComponent* AOmniPlayerController::GetCustomAbilitySystemComponent() const
{
	// ASC 的本体挂在玩家状态上, 这里只做一次判空转发
	if (const AExperiencePlayerState* ExperiencePS = GetPlayerState<AExperiencePlayerState>())
	{
		return ExperiencePS->GetCustomAbilitySystemComponent();
	}
	return nullptr;
}

void AOmniPlayerController::PostProcessInput(const float DeltaTime, const bool bGamePaused)
{
	// 输入组件只负责把按下/抬起记到 ASC 的输入队列里，真正的处理在这里每帧执行
	if (UCustomAbilitySystemComponent* ASC = GetCustomAbilitySystemComponent())
	{
		ASC->ProcessAbilityInput(DeltaTime, bGamePaused);
	}

	Super::PostProcessInput(DeltaTime, bGamePaused);
}
