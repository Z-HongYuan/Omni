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

		// 通过接口函数获取监听对象的队伍更改委托,并将自身的函数注册其中
		TeamInterface->GetTeamChangedDelegateChecked().AddDynamic(this, &ThisClass::OnWatchedAgentChangedTeam);

		bCouldSucceed = true;
	}

	// 广播一次，让用户获得当前状态
	OnTeamChanged.Broadcast(CurrentTeamIndex != INDEX_NONE, CurrentTeamIndex);

	// 我们无法绑定代表，因此永远不会有后续更新
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

	// 取消监听时,解绑
	if (ITeamAgentInterface* TeamInterface = TeamInterfacePtr.Get())
	{
		TeamInterface->GetTeamChangedDelegateChecked().RemoveAll(this);
	}
}

void UAsyncAction_ObserveTeam::OnWatchedAgentChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam)
{
	OnTeamChanged.Broadcast(NewTeam != INDEX_NONE, NewTeam);
}
