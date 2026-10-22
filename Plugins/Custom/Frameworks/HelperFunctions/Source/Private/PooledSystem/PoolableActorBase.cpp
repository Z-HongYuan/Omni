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
	SetActorHiddenInGame(false);

	SetActorEnableCollision(true);

	SetActorTickEnabled(true);

	// 唤醒网络休眠状态，刷新复制。
	SetNetDormancy(DORM_Awake);
	FlushNetDormancy();

	SetReplicateMovement(true);

	// 清除上一轮使用的寿命计时，避免复用后自动销毁。
	SetLifeSpan(0.0f);

	K2_OnPoolActivated();
}

void APoolableActorBase::OnPoolDeactivated()
{
	// 清理绑定到此对象的定时器，避免休眠后继续回调。
	GetWorldTimerManager().ClearAllTimersForObject(this);

	SetActorHiddenInGame(true);

	SetActorEnableCollision(false);

	SetActorTickEnabled(false);

	// 进入网络休眠。
	SetNetDormancy(DORM_DormantAll);

	SetReplicateMovement(false);

	// 清除上一轮使用的 Owner 和 Instigator。
	SetOwner(nullptr);
	SetInstigator(nullptr);

	K2_OnPoolDeactivated();
}
