// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "GameFramework/PlayerStart.h"
#include "GameplayTagContainer.h"
#include "ExpPlayerStart.generated.h"

#define UE_API EXPERIENCESYSTEM_API

class AController;
class UObject;

// 出生点的占用状态
enum class EExpPlayerStartLocationOccupancy : uint8
{
	// 完全空着，可以直接放
	Empty,
	// 有碰撞但可以找到一个传送落点
	Partial,
	// 放不下
	Full
};

/**
 * 体验系统使用的出生点基类
 */
UCLASS(MinimalAPI, Config = Game)
class AExpPlayerStart : public APlayerStart
{
	GENERATED_BODY()

public:
	UE_API AExpPlayerStart(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// 获取此出生点的 GameplayTag，供生成管理组件过滤
	const FGameplayTagContainer& GetGameplayTags() const { return StartPointTags; }

	// 判断能否放得下这个控制器即将生成的 Pawn
	UE_API EExpPlayerStartLocationOccupancy GetLocationOccupancy(AController* const ControllerPawnToFit) const;

	// 此出生点是否已经被某个控制器声明占用
	UE_API bool IsClaimed() const;

	// 如果尚未被占用，则为指定的控制器声明占用
	UE_API bool TryClaim(AController* OccupyingController);

protected:
	// 检查此出生点是否已经不再被占用（占用者已经离开且位置空出来了就释放）
	UE_API void CheckUnclaimed();

	// 当前声明占用此出生点的控制器
	UPROPERTY(Transient)
	TObjectPtr<AController> ClaimingController = nullptr;

	// 检查占用是否过期的时间间隔
	UPROPERTY(EditDefaultsOnly, Category = "Player Start Claiming")
	float ExpirationCheckInterval = 1.f;

	// 用于识别此出生点的标签
	UPROPERTY(EditAnywhere)
	FGameplayTagContainer StartPointTags;

	// 用于跟踪过期检查的定时器句柄
	FTimerHandle ExpirationTimerHandle;
};

#undef UE_API
