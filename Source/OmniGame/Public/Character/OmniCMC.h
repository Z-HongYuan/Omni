// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "GameFramework/CharacterMovementComponent.h"
#include "OmniCMC.generated.h"

#define UE_API OMNIGAME_API

/** 由移动组件主动更新的地面快照；距离以胶囊底部为起点，单位为厘米。 */
USTRUCT(BlueprintType)
struct FOmniCharacterGroundInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Omni|CharacterMovement")
	FHitResult GroundHitResult;

	// -1 表示尚未更新或没有有效来源；未命中时为检测上限，并非实际地面距离。
	UPROPERTY(BlueprintReadOnly, Category = "Omni|CharacterMovement")
	float GroundDistance = -1.0f;
};

/** 项目移动组件：保留引擎移动行为，只补充供动画等消费者读取的地面信息。 */
UCLASS(MinimalAPI, Config = Game)
class UOmniCMC : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	UE_API UOmniCMC(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UE_API virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	UE_API virtual void TickCharacterPose(float DeltaTime) override;

	// CMC 在游戏线程生成，读取不触发检测。ABP 通过 Property Access 在游戏线程采样后使用。
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Omni|CharacterMovement")
	FOmniCharacterGroundInfo GroundInfo;

private:
	void UpdateGroundInfo();
};

#undef UE_API
