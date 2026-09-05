// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Core/TeamAgentInterface.h"

#include "LogCustomTeam.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TeamAgentInterface)

void ITeamAgentInterface::BroadcastTeamChanged_Conditional(TScriptInterface<ITeamAgentInterface> This, FGenericTeamId OldTeamID, FGenericTeamId NewTeamID)
{
	if (OldTeamID != NewTeamID)
	{
		const int32 OldTeamIndex = GenericTeamIdToInteger(OldTeamID);
		const int32 NewTeamIndex = GenericTeamIdToInteger(NewTeamID);

		UObject* ThisObj = This.GetObject();
		// UE_LOG(LogCustomTeam, Verbose, TEXT("[%s] %s assigned team %d"), *GetClientServerContextString(ThisObj), *GetPathNameSafe(ThisObj), NewTeamIndex);

		This.GetInterface()->GetTeamChangedDelegateChecked().Broadcast(ThisObj, OldTeamIndex, NewTeamIndex);
	}
}
