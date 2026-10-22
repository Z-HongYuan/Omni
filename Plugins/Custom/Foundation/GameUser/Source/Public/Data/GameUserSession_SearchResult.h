// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Data/GameUserTypes.h"
#include "OnlineSessionSettings.h"
#include "UObject/Object.h"
#include "GameUserSession_SearchResult.generated.h"

#define UE_API GAMEUSER_API

/** 从在线系统返回的描述可加入游戏会话的结果对象。 */
UCLASS(MinimalAPI, BlueprintType)
class UGameUserSession_SearchResult : public UObject
{
	GENERATED_BODY()

public:
	/** 返回会话的内部描述，不适合人类阅读。 */
	UFUNCTION(BlueprintCallable, Category=Session)
	UE_API FString GetDescription() const;

	/** 获取任意字符串设置，如果设置不存在则 bFoundValue 为 false。 */
	UFUNCTION(BlueprintPure, Category=Sessions)
	UE_API void GetStringSetting(FName Key, FString& Value, bool& bFoundValue) const;

	/** 获取任意整数设置，如果设置不存在则 bFoundValue 为 false。 */
	UFUNCTION(BlueprintPure, Category = Sessions)
	UE_API void GetIntSetting(FName Key, int32& Value, bool& bFoundValue) const;

	/** 剩余的私有连接名额。 */
	UFUNCTION(BlueprintPure, Category=Sessions)
	UE_API int32 GetNumOpenPrivateConnections() const;

	/** 剩余的公共连接名额。 */
	UFUNCTION(BlueprintPure, Category=Sessions)
	UE_API int32 GetNumOpenPublicConnections() const;

	/** 公共连接总容量，包括已占用的名额。 */
	UFUNCTION(BlueprintPure, Category = Sessions)
	UE_API int32 GetMaxPublicConnections() const;

	/** 到该会话的 Ping，单位为毫秒；MAX_QUERY_PING 表示不可达。 */
	UFUNCTION(BlueprintPure, Category=Sessions)
	UE_API int32 GetPingInMs() const;

public:
	/** 底层 OSS 返回的会话搜索结果值。 */
	FOnlineSessionSearchResult Result;
};

#undef UE_API

/**
 * 本地用户从平台界面等外部来源请求加入会话时广播，由游戏决定如何处理。
 * LocalPlatformUserId 为平台用户 ID；请求失败时 RequestedSession 可为空。
 * RequestedSessionResult 描述处理加入请求时的结果。
 */
DECLARE_MULTICAST_DELEGATE_ThreeParams(FGameUserSessionOnUserRequestedSession, const FPlatformUserId& /*LocalPlatformUserId*/, UGameUserSession_SearchResult* /*RequestedSession*/, const FOnlineResultInformation& /*RequestedSessionResult*/);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FGameUserSessionOnUserRequestedSession_Dynamic, const FPlatformUserId&, LocalPlatformUserId, UGameUserSession_SearchResult*, RequestedSession, const FOnlineResultInformation&,
                                               RequestedSessionResult);
