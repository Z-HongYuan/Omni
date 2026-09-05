// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Data/GameUserTypes.h"
#include "OnlineSessionSettings.h"
#include "UObject/Object.h"
#include "GameUserSession_SearchResult.generated.h"

#define UE_API GAMEUSER_API

/** 从在线系统返回的描述可加入游戏会话的结果对象 */
UCLASS(MinimalAPI, BlueprintType)
class UGameUserSession_SearchResult : public UObject
{
	GENERATED_BODY()

public:
	/** 返回会话的内部描述,不适合人类阅读 */
	UFUNCTION(BlueprintCallable, Category=Session)
	UE_API FString GetDescription() const;

	/** 获取任意字符串设置,如果设置不存在则 bFoundValue 为 false */
	UFUNCTION(BlueprintPure, Category=Sessions)
	UE_API void GetStringSetting(FName Key, FString& Value, bool& bFoundValue) const;

	/** 获取任意整数设置,如果设置不存在则 bFoundValue 为 false */
	UFUNCTION(BlueprintPure, Category = Sessions)
	UE_API void GetIntSetting(FName Key, int32& Value, bool& bFoundValue) const;

	/** 可用的专用连接数 */
	UFUNCTION(BlueprintPure, Category=Sessions)
	UE_API int32 GetNumOpenPrivateConnections() const;

	/** 可用的公共连接数 */
	UFUNCTION(BlueprintPure, Category=Sessions)
	UE_API int32 GetNumOpenPublicConnections() const;

	/** 可用的最大公共连接数,包括已占用的连接 */
	UFUNCTION(BlueprintPure, Category = Sessions)
	UE_API int32 GetMaxPublicConnections() const;

	/** 到搜索结果对象的 Ping 值,MAX_QUERY_PING 表示不可达 */
	UFUNCTION(BlueprintPure, Category=Sessions)
	UE_API int32 GetPingInMs() const;

public:
	/** 指向平台特定实现的指针 */
	FOnlineSessionSearchResult Result;
};

#undef UE_API

/**
 * 当本地用户从外部来源(例如平台覆盖层)请求加入会话时触发的事件。
 * 通常,游戏应将玩家转入该会话。
 * @param LocalPlatformUserId 接受邀请的本地用户 ID。由于用户可能尚未登录,因此这是平台用户 ID。
 * @param RequestedSession 请求的会话。如果处理请求时出错,可能为 null。
 * @param RequestedSessionResult 请求会话处理的结果
 */
DECLARE_MULTICAST_DELEGATE_ThreeParams(FGameUserSessionOnUserRequestedSession, const FPlatformUserId& /*LocalPlatformUserId*/, UGameUserSession_SearchResult* /*RequestedSession*/, const FOnlineResultInformation& /*RequestedSessionResult*/);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FGameUserSessionOnUserRequestedSession_Dynamic, const FPlatformUserId&, LocalPlatformUserId, UGameUserSession_SearchResult*, RequestedSession, const FOnlineResultInformation&,
                                               RequestedSessionResult);
