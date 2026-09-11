// Copyright © 2026 张鸿源. All Rights Reserved.

#include "Data/GameUserSession_SearchResult.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameUserSession_SearchResult)

FString UGameUserSession_SearchResult::GetDescription() const
{
	return Result.GetSessionIdStr();
}

void UGameUserSession_SearchResult::GetStringSetting(FName Key, FString& Value, bool& bFoundValue) const
{
	bFoundValue = Result.Session.SessionSettings.Get<FString>(Key, /*out*/ Value);
}

void UGameUserSession_SearchResult::GetIntSetting(FName Key, int32& Value, bool& bFoundValue) const
{
	bFoundValue = Result.Session.SessionSettings.Get<int32>(Key, /*out*/ Value);
}

int32 UGameUserSession_SearchResult::GetNumOpenPrivateConnections() const
{
	return Result.Session.NumOpenPrivateConnections;
}

int32 UGameUserSession_SearchResult::GetNumOpenPublicConnections() const
{
	return Result.Session.NumOpenPublicConnections;
}

int32 UGameUserSession_SearchResult::GetMaxPublicConnections() const
{
	return Result.Session.SessionSettings.NumPublicConnections;
}

int32 UGameUserSession_SearchResult::GetPingInMs() const
{
	return Result.PingInMs;
}
