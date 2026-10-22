// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "GenericTeamAgentInterface.h"
#include "UObject/Interface.h"
#include "TeamAgentInterface.generated.h"

#define UE_API CUSTOMTEAM_API

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnTeamIndexChangedDelegate, UObject*, ObjectChangingTeam, int32, OldTeamID, int32, NewTeamID);

/** 将通用队伍 ID 转为整数；NoTeam 转为 INDEX_NONE。 */
UE_API inline int32 GenericTeamIdToInteger(FGenericTeamId ID)
{
	return (ID == FGenericTeamId::NoTeam) ? INDEX_NONE : static_cast<int32>(ID);
}

/** 将整数转为通用队伍 ID；INDEX_NONE 转为 NoTeam。 */
UE_API inline FGenericTeamId IntegerToGenericTeamId(int32 ID)
{
	return (ID == INDEX_NONE) ? FGenericTeamId::NoTeam : FGenericTeamId(static_cast<uint8>(ID));
}

/** 提供队伍归属及队伍变化通知的参与者接口。 */
UINTERFACE(MinimalAPI)
class UTeamAgentInterface : public UGenericTeamAgentInterface
{
	GENERATED_BODY()
};

/** 提供队伍归属及队伍变化通知的参与者接口。 */
class ITeamAgentInterface : public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	/** 返回队伍变化委托；未提供委托的实现返回 nullptr。 */
	UE_API virtual FOnTeamIndexChangedDelegate* GetOnTeamIndexChangedDelegate() { return nullptr; }

	/** 仅当队伍 ID 变化时广播；接口实现必须提供有效的变化委托。 */
	static UE_API void BroadcastTeamChanged_Conditional(TScriptInterface<ITeamAgentInterface> This, FGenericTeamId OldTeamID, FGenericTeamId NewTeamID);

	/** 取得队伍变化委托，并断言接口实现已提供该委托。 */
	FOnTeamIndexChangedDelegate& GetTeamChangedDelegateChecked()
	{
		FOnTeamIndexChangedDelegate* Result = GetOnTeamIndexChangedDelegate();
		check(Result);
		return *Result;
	}
};

#undef UE_API
