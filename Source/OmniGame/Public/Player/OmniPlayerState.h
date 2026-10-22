// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Gameplay/ExpPlayerState.h"
#include "OmniPlayerState.generated.h"

#define UE_API OMNIGAME_API

class UExtHealthSet;

/**
 * 项目玩家状态，沿用父类的 ASC、PawnData 复制和体验就绪后的能力授予。
 * 持有玩家 ASC 与生命属性集；Pawn 初始化组件通过 PawnExtension 绑定当前 Avatar。
 */
UCLASS(MinimalAPI, Config = Game)
class AOmniPlayerState : public AExpPlayerState
{
	GENERATED_BODY()

public:
	UE_API AOmniPlayerState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	UE_API virtual void OnExperienceLoaded(const UExpDefinition* CurrentExperience) override;

private:
	/** 与 PlayerState 和 ASC 同生命周期的生命属性集，AbilitySet 无需重复添加。 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Omni|Health", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UExtHealthSet> HealthSet;
};

#undef UE_API
