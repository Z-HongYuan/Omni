// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Engine/CancellableAsyncAction.h"
#include "UObject/WeakInterfacePtr.h"
#include "AsyncAction_ObserveTeam.generated.h"

#define UE_API CUSTOMTEAM_API

class ITeamAgentInterface;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FTeamObservedAsyncDelegate, bool, bTeamSet, int32, TeamId);

/** 监听队伍接口的变化，并在取消或销毁时解除委托绑定。 */
UCLASS(MinimalAPI)
class UAsyncAction_ObserveTeam : public UCancellableAsyncAction
{
	GENERATED_BODY()

public:
	/**
	 * 创建队伍监听任务；Activate 时广播当前状态，并持续监听变化。
	 * 接口实现必须提供有效委托；目标不支持接口或已失效时，首次广播后结束。
	 * TeamId 为 INDEX_NONE 时表示未分配队伍，bTeamSet 为 false。
	 */
	UFUNCTION(BlueprintCallable, meta=(BlueprintInternalUseOnly="true", Keywords="Watch"))
	static UAsyncAction_ObserveTeam* ObserveTeam(UObject* TeamAgent);

	//~UBlueprintAsyncActionBase interface
	virtual void Activate() override;
	/** 通过基类取消任务，由 SetReadyToDestroy 解除队伍委托绑定。 */
	virtual void Cancel() override;
	virtual void SetReadyToDestroy() override;
	//~End of UBlueprintAsyncActionBase interface

	UPROPERTY(BlueprintAssignable)
	FTeamObservedAsyncDelegate OnTeamChanged;

private:
	UFUNCTION()
	void OnWatchedAgentChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam);

	TWeakInterfacePtr<ITeamAgentInterface> TeamInterfacePtr;
};
#undef UE_API
