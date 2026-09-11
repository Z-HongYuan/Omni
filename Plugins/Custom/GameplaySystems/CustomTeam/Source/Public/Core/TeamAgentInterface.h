// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "GenericTeamAgentInterface.h"
#include "UObject/Interface.h"
#include "TeamAgentInterface.generated.h"

#define UE_API CUSTOMTEAM_API

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnTeamIndexChangedDelegate, UObject*, ObjectChangingTeam, int32, OldTeamID, int32, NewTeamID);

// 将通用队伍ID转换为整数
UE_API inline int32 GenericTeamIdToInteger(FGenericTeamId ID)
{
	return (ID == FGenericTeamId::NoTeam) ? INDEX_NONE : static_cast<int32>(ID);
}

// 将整数转换为通用队伍ID
UE_API inline FGenericTeamId IntegerToGenericTeamId(int32 ID)
{
	return (ID == INDEX_NONE) ? FGenericTeamId::NoTeam : FGenericTeamId(static_cast<uint8>(ID));
}

/**
 * 可与团队关联的参与者接口
 */
UINTERFACE(MinimalAPI)
class UTeamAgentInterface : public UGenericTeamAgentInterface
{
	GENERATED_BODY()
};

/**
 * 可与团队关联的参与者接口
 */
class ITeamAgentInterface : public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	// 获取团队变化委托
	UE_API virtual FOnTeamIndexChangedDelegate* GetOnTeamIndexChangedDelegate() { return nullptr; }

	// 广播团队变化，仅当团队ID发生变化时才广播
	static UE_API void BroadcastTeamChanged_Conditional(TScriptInterface<ITeamAgentInterface> This, FGenericTeamId OldTeamID, FGenericTeamId NewTeamID);

	// 获取团队变化委托，确保委托存在, 接口内部使用
	FOnTeamIndexChangedDelegate& GetTeamChangedDelegateChecked()
	{
		FOnTeamIndexChangedDelegate* Result = GetOnTeamIndexChangedDelegate();
		check(Result);
		return *Result;
	}
};

#undef UE_API
