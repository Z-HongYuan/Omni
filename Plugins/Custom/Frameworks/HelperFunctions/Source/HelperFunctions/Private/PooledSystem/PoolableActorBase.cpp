// Copyright © 2026 张鸿源. All Rights Reserved.


#include "PooledSystem/PoolableActorBase.h"

#include "TimerManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PoolableActorBase)

APoolableActorBase::APoolableActorBase()
{
	PrimaryActorTick.bCanEverTick = false;
}

void APoolableActorBase::OnPoolActivated()
{
	// 显示 Actor
	SetActorHiddenInGame(false);

	// 开启 碰撞
	SetActorEnableCollision(true);

	// 开启 Tick
	SetActorTickEnabled(true);

	// 网络活跃
	SetNetDormancy(DORM_Awake);
	FlushNetDormancy();

	// 启用网络同步
	SetReplicateMovement(true);

	// 设置生命周期
	SetLifeSpan(0.0f);

	// 蓝图回调
	K2_OnPoolActivated();
}

void APoolableActorBase::OnPoolDeactivated()
{
	// 清理 Timer 委托
	GetWorldTimerManager().ClearAllTimersForObject(this);

	// 隐藏 Actor
	SetActorHiddenInGame(true);

	// 禁用碰撞
	SetActorEnableCollision(false);

	// 禁用 Tick
	SetActorTickEnabled(false);

	// 网络休眠
	SetNetDormancy(DORM_DormantAll);

	// 取消网络同步
	SetReplicateMovement(false);

	// 清空 Owner 和 Instigator
	SetOwner(nullptr);
	SetInstigator(nullptr);

	// 蓝图回调
	K2_OnPoolDeactivated();
}
