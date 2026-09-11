// Copyright © 2026 张鸿源. All Rights Reserved.


#include "Core/TeamHelper.h"

#include "LogCustomTeam.h"
#include "Core/TeamSubsystem.h"
#include "Data/TeamDisplayAssetBase.h"
#include "Engine/Engine.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TeamHelper)

void UTeamHelper::FindTeamFromObject(const UObject* Agent, bool& bIsPartOfTeam, int32& TeamId, UTeamDisplayAssetBase*& DisplayAsset, bool bLogIfNotSet)
{
	bIsPartOfTeam = false;
	TeamId = INDEX_NONE;
	DisplayAsset = nullptr;

	if (UWorld* World = GEngine->GetWorldFromContextObject(Agent, EGetWorldErrorMode::LogAndReturnNull))
	{
		if (UTeamSubsystem* TeamSubsystem = World->GetSubsystem<UTeamSubsystem>())
		{
			TeamId = TeamSubsystem->FindTeamFromObject(Agent);
			if (TeamId != INDEX_NONE)
			{
				bIsPartOfTeam = true;

				DisplayAsset = TeamSubsystem->GetTeamDisplayAsset(TeamId, INDEX_NONE);

				if ((DisplayAsset == nullptr) && bLogIfNotSet)
				{
					UE_LOG(LogCustomTeam, Log, TEXT("FindTeamFromObject(%s) called too early (found team %d but no display asset set yet"), *GetPathNameSafe(Agent), TeamId);
				}
			}
		}
		else
		{
			UE_LOG(LogCustomTeam, Error, TEXT("FindTeamFromObject(%s) failed: Team subsystem does not exist yet"), *GetPathNameSafe(Agent));
		}
	}
}

UTeamDisplayAssetBase* UTeamHelper::GetTeamDisplayAsset(const UObject* WorldContextObject, int32 TeamId)
{
	UTeamDisplayAssetBase* Result = nullptr;
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		if (UTeamSubsystem* TeamSubsystem = World->GetSubsystem<UTeamSubsystem>())
		{
			return TeamSubsystem->GetTeamDisplayAsset(TeamId, INDEX_NONE);
		}
	}
	return Result;
}
