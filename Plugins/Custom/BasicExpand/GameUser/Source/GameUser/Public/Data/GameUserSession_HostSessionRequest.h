// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "UObject/Object.h"
#include "UObject/PrimaryAssetId.h"
#include "GameUserSession_HostSessionRequest.generated.h"

#define UE_API GAMEUSER_API

/** 指定游戏会话应使用的在线功能和连接方式 */
UENUM(BlueprintType)
enum class EGameUserSessionOnlineMode : uint8
{
	Offline,
	LAN,
	Online
};

/** 一个请求对象,用于存储在托管游戏会话时使用的参数 */
UCLASS(MinimalAPI, BlueprintType)
class UGameUserSession_HostSessionRequest : public UObject
{
	GENERATED_BODY()

public:
	/** 指示该会话是完整的在线会话还是其他类型 */
	UPROPERTY(BlueprintReadWrite, Category=Session)
	EGameUserSessionOnlineMode OnlineMode;

	/** 如果为 true,则此请求应在可用时创建由玩家托管的 Lobby */
	UPROPERTY(BlueprintReadWrite, Category = Session)
	bool bUseLobbies;

	/** 如果为 true,则此请求应在可用时创建启用语音聊天的 Lobby */
	UPROPERTY(BlueprintReadWrite, Category = Session)
	bool bUseLobbiesVoiceChat;

	/** 如果为 true,则此请求应创建一个会出现在用户在线状态信息中的会话 */
	UPROPERTY(BlueprintReadWrite, Category = Session)
	bool bUsePresence;

	/** 匹配期间用于指定此游戏模式类型的字符串 */
	UPROPERTY(BlueprintReadWrite, Category=Session)
	FString ModeNameForAdvertisement;

	/** 游戏开始时将加载的地图,这必须是有效的 Primary Asset 顶级地图 */
	UPROPERTY(BlueprintReadWrite, Category=Session, meta=(AllowedTypes="World"))
	FPrimaryAssetId MapID;

	/** 作为 URL 选项传递给游戏的额外参数 */
	UPROPERTY(BlueprintReadWrite, Category=Session)
	TMap<FString, FString> ExtraArgs;

	/** 每个游戏会话允许的最大玩家数 */
	UPROPERTY(BlueprintReadWrite, Category=Session)
	int32 MaxPlayerCount = 16;

public:
	/** 返回实际应使用的最大玩家数,可在子类中覆盖 */
	UE_API virtual int32 GetMaxPlayers() const;

	/** 返回游戏期间将使用的完整地图名称 */
	UE_API virtual FString GetMapName() const;

	/** 构造将传递给 ServerTravel 的完整 URL */
	UE_API virtual FString ConstructTravelURL() const;

	/** 如果此请求有效则返回 true,否则返回 false 并记录错误 */
	UE_API virtual bool ValidateAndLogErrors(FText& OutError) const;
};

#undef UE_API
