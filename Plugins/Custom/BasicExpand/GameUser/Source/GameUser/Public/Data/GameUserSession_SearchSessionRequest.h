// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Data/GameUserSession_HostSessionRequest.h"
#include "UObject/Object.h"
#include "GameUserSession_SearchSessionRequest.generated.h"

#define UE_API GAMEUSER_API

class UGameUserSession_SearchResult;

/** 会话搜索完成时调用的委托 */
DECLARE_MULTICAST_DELEGATE_TwoParams(FGameUserSession_FindSessionsFinished, bool bSucceeded, const FText& ErrorMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGameUserSession_FindSessionsFinishedDynamic, bool, bSucceeded, FText, ErrorMessage);

/** 描述会话搜索的请求对象,搜索完成后此对象将被更新 */
UCLASS(MinimalAPI, BlueprintType)
class UGameUserSession_SearchSessionRequest : public UObject
{
	GENERATED_BODY()

public:
	/** 指示这是在查找完整的在线游戏还是其他类型(如 LAN) */
	UPROPERTY(BlueprintReadWrite, Category = Session)
	EGameUserSessionOnlineMode OnlineMode;

	/** 如果为 true,则此请求应在可用时查找玩家托管的 Lobby,为 false 时仅搜索已注册的服务器会话 */
	UPROPERTY(BlueprintReadWrite, Category = Session)
	bool bUseLobbies;

	/** 所有找到的会话列表,在调用 OnSearchFinished 时将有效 */
	UPROPERTY(BlueprintReadOnly, Category=Session)
	TArray<TObjectPtr<UGameUserSession_SearchResult>> Results;

	/** 会话搜索完成时调用的原生委托 */
	FGameUserSession_FindSessionsFinished OnSearchFinished;

	/** 由子系统调用以执行完成委托 */
	UE_API void NotifySearchFinished(bool bSucceeded, const FText& ErrorMessage);

private:
	/** 会话搜索完成时调用的委托 */
	UPROPERTY(BlueprintAssignable, Category = "Events", meta = (DisplayName = "On Search Finished", AllowPrivateAccess = true))
	FGameUserSession_FindSessionsFinishedDynamic K2_OnSearchFinished;
};

#undef UE_API
