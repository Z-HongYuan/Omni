// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Tasks/AsyncAction_ObserveTeam.h"

#include "Core/TeamAgentInterface.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AsyncAction_ObserveTeam)

UAsyncAction_ObserveTeam* UAsyncAction_ObserveTeam::ObserveTeam(UObject* TeamAgent)
{
	UAsyncAction_ObserveTeam* Action = nullptr;

	if (TeamAgent != nullptr)
	{
		Action = NewObject<UAsyncAction_ObserveTeam>();
		Action->TeamInterfacePtr = TWeakInterfacePtr<ITeamAgentInterface>(TeamAgent);
		// Action->TeamInterfacePtr = TScriptInterface<ITeamAgentInterface>(TeamAgent);
		Action->RegisterWithGameInstance(TeamAgent);
	}

	return Action;
}

void UAsyncAction_ObserveTeam::Activate()
{
	bool bCouldSucceed = false;
	int32 CurrentTeamIndex = INDEX_NONE;

	if (ITeamAgentInterface* TeamInterface = TeamInterfacePtr.Get())
	{
		CurrentTeamIndex = GenericTeamIdToInteger(TeamInterface->GetGenericTeamId());

		// 先绑定后续变化，再广播当前状态。
		TeamInterface->GetTeamChangedDelegateChecked().AddDynamic(this, &ThisClass::OnWatchedAgentChangedTeam);

		bCouldSucceed = true;
	}

	// 即使无法持续监听，也先广播当前可解析的队伍状态。
	OnTeamChanged.Broadcast(CurrentTeamIndex != INDEX_NONE, CurrentTeamIndex);

	// 目标接口不可取得时，不再保留这个异步任务。
	if (!bCouldSucceed)
	{
		SetReadyToDestroy();
	}
}

void UAsyncAction_ObserveTeam::Cancel()
{
	Super::Cancel();
}

void UAsyncAction_ObserveTeam::SetReadyToDestroy()
{
	Super::SetReadyToDestroy();

	// 取消或销毁时解除绑定；弱接口引用不延长目标生命周期。
	if (ITeamAgentInterface* TeamInterface = TeamInterfacePtr.Get())
	{
		TeamInterface->GetTeamChangedDelegateChecked().RemoveAll(this);
	}
}

void UAsyncAction_ObserveTeam::OnWatchedAgentChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam)
{
	OnTeamChanged.Broadcast(NewTeam != INDEX_NONE, NewTeam);
}
