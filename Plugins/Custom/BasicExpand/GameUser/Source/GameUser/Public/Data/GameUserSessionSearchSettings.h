// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Data/GameUserSession_SearchSessionRequest.h"
#include "OnlineSessionSettings.h"
#include "UObject/GCObject.h"
#include "UObject/ObjectPtr.h"

/** 用于标识在线会话模板版本的设置键,定义于 GameUserSessionSubsystem.cpp */
extern FName SETTING_ONLINESUBSYSTEM_VERSION;

/** 搜索设置基类,持有搜索请求对象引用并参与垃圾回收 */
class FGameOnlineSearchSettingsBase : public FGCObject
{
public:
	FGameOnlineSearchSettingsBase(UGameUserSession_SearchSessionRequest* InSearchRequest);

	virtual ~FGameOnlineSearchSettingsBase() { ; }

	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;

	virtual FString GetReferencerName() const override;

public:
	TObjectPtr<UGameUserSession_SearchSessionRequest> SearchRequest = nullptr;
};

/** 托管游戏会话时使用的在线会话设置 */
class FGameUserSession_OnlineSessionSettings : public FOnlineSessionSettings
{
public:
	FGameUserSession_OnlineSessionSettings(bool bIsLAN = false, bool bIsPresence = false, int32 MaxNumPlayers = 4);

	virtual ~FGameUserSession_OnlineSessionSettings() { ; }
};

/** OSS v1 的在线会话搜索设置 */
class FGameOnlineSearchSettingsOSSv1 : public FOnlineSessionSearch, public FGameOnlineSearchSettingsBase
{
public:
	FGameOnlineSearchSettingsOSSv1(UGameUserSession_SearchSessionRequest* InSearchRequest);

	virtual ~FGameOnlineSearchSettingsOSSv1() { ; }
};
