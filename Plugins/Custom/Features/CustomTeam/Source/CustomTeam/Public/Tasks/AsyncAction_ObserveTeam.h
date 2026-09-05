// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Engine/CancellableAsyncAction.h"
#include "UObject/WeakInterfacePtr.h"
#include "AsyncAction_ObserveTeam.generated.h"

#define UE_API CUSTOMTEAM_API

class ITeamAgentInterface;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FTeamObservedAsyncDelegate, bool, bTeamSet, int32, TeamId);

/**
 * 监视团队变化状态
 */
UCLASS(MinimalAPI)
class UAsyncAction_ObserveTeam : public UCancellableAsyncAction
{
	GENERATED_BODY()

public:
	//监视指定团队代理上的团队更改
	//-它将立即执行一次，以分配当前的团队任务
	//-对于任何可能属于团队的东西（实现ITeagAgentInterface），
	//它还将监听未来团队任务的变化
	//如果广播时，团队ID为INDEX_NONE，则表示未设置团队,则bool为false
	UFUNCTION(BlueprintCallable, meta=(BlueprintInternalUseOnly="true", Keywords="Watch"))
	static UAsyncAction_ObserveTeam* ObserveTeam(UObject* TeamAgent);

	//~UBlueprintAsyncActionBase interface
	virtual void Activate() override;
	virtual void Cancel() override; //取消的时候会自动调用 SetReadyToDestroy()
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
