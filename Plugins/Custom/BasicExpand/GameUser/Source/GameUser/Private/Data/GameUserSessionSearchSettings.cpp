// Copyright © 2026 张鸿源. All Rights Reserved.

#include "Data/GameUserSessionSearchSettings.h"
#include "Online/OnlineSessionNames.h"

FName SETTING_ONLINESUBSYSTEM_VERSION(TEXT("OSSv1"));

FGameOnlineSearchSettingsBase::FGameOnlineSearchSettingsBase(UGameUserSession_SearchSessionRequest* InSearchRequest)
{
	SearchRequest = InSearchRequest;
}

void FGameOnlineSearchSettingsBase::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(SearchRequest);
}

FString FGameOnlineSearchSettingsBase::GetReferencerName() const
{
	static const FString NameString = TEXT("FGameOnlineSearchSettings");
	return NameString;
}

FGameUserSession_OnlineSessionSettings::FGameUserSession_OnlineSessionSettings(bool bIsLAN, bool bIsPresence, int32 MaxNumPlayers)
{
	NumPublicConnections = MaxNumPlayers;
	if (NumPublicConnections < 0)
	{
		NumPublicConnections = 0;
	}
	NumPrivateConnections = 0;
	bIsLANMatch = bIsLAN;
	bShouldAdvertise = true;
	bAllowJoinInProgress = true;
	bAllowInvites = true;
	bUsesPresence = bIsPresence;
	bAllowJoinViaPresence = true;
	bAllowJoinViaPresenceFriendsOnly = false;
}

FGameOnlineSearchSettingsOSSv1::FGameOnlineSearchSettingsOSSv1(UGameUserSession_SearchSessionRequest* InSearchRequest)
	: FGameOnlineSearchSettingsBase(InSearchRequest)
{
	bIsLanQuery = (InSearchRequest->OnlineMode == EGameUserSessionOnlineMode::LAN);
	MaxSearchResults = 10;
	PingBucketSize = 50;

	QuerySettings.Set(SETTING_ONLINESUBSYSTEM_VERSION, true, EOnlineComparisonOp::Equals);

	if (InSearchRequest->bUseLobbies)
	{
		QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);
	}
}
