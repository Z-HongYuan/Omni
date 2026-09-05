// Copyright © 2026 张鸿源. All Rights Reserved.

#include "Data/GameUserSession_HostSessionRequest.h"
#include "AssetRegistry/AssetData.h"
#include "Engine/AssetManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameUserSession_HostSessionRequest)

int32 UGameUserSession_HostSessionRequest::GetMaxPlayers() const
{
	return MaxPlayerCount;
}

FString UGameUserSession_HostSessionRequest::GetMapName() const
{
	FAssetData MapAssetData;
	if (UAssetManager::Get().GetPrimaryAssetData(MapID, /*out*/ MapAssetData))
	{
		return MapAssetData.PackageName.ToString();
	}

	return FString();
}

FString UGameUserSession_HostSessionRequest::ConstructTravelURL() const
{
	FString CombinedExtraArgs;

	if (OnlineMode == EGameUserSessionOnlineMode::LAN)
	{
		CombinedExtraArgs += TEXT("?bIsLanMatch");
	}

	if (OnlineMode != EGameUserSessionOnlineMode::Offline)
	{
		CombinedExtraArgs += TEXT("?listen");
	}

	for (const auto& KVP : ExtraArgs)
	{
		if (!KVP.Key.IsEmpty())
		{
			if (KVP.Value.IsEmpty())
			{
				CombinedExtraArgs += FString::Printf(TEXT("?%s"), *KVP.Key);
			}
			else
			{
				CombinedExtraArgs += FString::Printf(TEXT("?%s=%s"), *KVP.Key, *KVP.Value);
			}
		}
	}

	//bIsRecordingDemo ? TEXT("?DemoRec") : TEXT(""));

	return FString::Printf(TEXT("%s%s"),
	                       *GetMapName(),
	                       *CombinedExtraArgs);
}

bool UGameUserSession_HostSessionRequest::ValidateAndLogErrors(FText& OutError) const
{
#if WITH_SERVER_CODE
	if (GetMapName().IsEmpty())
	{
		OutError = FText::Format(NSLOCTEXT("NetworkErrors", "InvalidMapFormat", "Can't find asset data for MapID {0}, hosting request failed."), FText::FromString(MapID.ToString()));
		return false;
	}

	return true;
#else
	// 客户端构建版本仅用于连接专用服务器,默认缺少创建主机(托管)会话的代码
	// 你可以在子类中更改此行为,以处理类似教程之类的场景
	OutError = NSLOCTEXT("NetworkErrors", "ClientBuildCannotHost", "Client builds cannot host game sessions.");
	return false;
#endif
}
