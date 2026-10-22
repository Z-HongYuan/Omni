// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "GameFramework/PlayerStart.h"
#include "GameplayTagContainer.h"
#include "ExpPlayerStart.generated.h"

#define UE_API EXPERIENCESYSTEM_API

class AController;
class UObject;

/** 出生点对待生成 Pawn 的碰撞容纳状态。 */
enum class EExpPlayerStartLocationOccupancy : uint8
{
	/** 当前位置无阻挡，可直接放置 Pawn。 */
	Empty,
	/** 当前位置有阻挡，但可找到附近的传送落点。 */
	Partial,
	/** 无法容纳 Pawn，或当前无权威端检测条件。 */
	Full
};

/** 提供碰撞占用检测、声明占用和筛选标签的体验出生点。 */
UCLASS(MinimalAPI, Config = Game)
class AExpPlayerStart : public APlayerStart
{
	GENERATED_BODY()

public:
	UE_API AExpPlayerStart(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** 返回出生点标签，供选点策略过滤。 */
	const FGameplayTagContainer& GetGameplayTags() const { return StartPointTags; }

	/** 在权威端按控制器实际使用的 Pawn 类型检测碰撞空间；客户端返回 Full。 */
	UE_API EExpPlayerStartLocationOccupancy GetLocationOccupancy(AController* const ControllerPawnToFit) const;

	/** 是否存在声明占用的控制器，与碰撞占用检测分开记录。 */
	UE_API bool IsClaimed() const;

	/** 尚未声明占用时记录控制器并启动检查计时，不进行碰撞检测。 */
	UE_API bool TryClaim(AController* OccupyingController);

protected:
	/** 控制器已有 Pawn 且出生位置无阻挡时，释放声明占用并停止计时。 */
	UE_API void CheckUnclaimed();

	/** 当前声明占用出生点的控制器。 */
	UPROPERTY(Transient)
	TObjectPtr<AController> ClaimingController = nullptr;

	/** 检查声明占用能否释放的间隔，单位为秒。 */
	UPROPERTY(EditDefaultsOnly, Category = "Player Start Claiming")
	float ExpirationCheckInterval = 1.f;

	/** 供选点策略识别出生点的标签。 */
	UPROPERTY(EditAnywhere)
	FGameplayTagContainer StartPointTags;

	/** 声明占用释放检查的定时器句柄。 */
	FTimerHandle ExpirationTimerHandle;
};

#undef UE_API
