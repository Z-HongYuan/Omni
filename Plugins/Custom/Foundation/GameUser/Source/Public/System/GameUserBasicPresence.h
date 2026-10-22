// Copyright © 2026 张鸿源. All Rights Reserved.
#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "GameUserBasicPresence.generated.h"

#define UE_API GAMEUSER_API

class UGameUserSessionSubsystem;
enum class EGameUserSessionInformationState : uint8;

/** 将会话状态、游戏模式和地图名称映射到在线状态接口；专用服务器不推送。 */
UCLASS(MinimalAPI, BlueprintType, Config = Engine)
class UGameUserBasicPresence : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UE_API virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	UE_API virtual void Deinitialize() override;

	UE_API void OnNotifySessionInformationChanged(EGameUserSessionInformationState SessionStatus, const FString& GameMode, const FString& MapName);
	UE_API FString SessionStateToBackendKey(EGameUserSessionInformationState SessionStatus);

	/** 是否启用基于会话信息的在线状态推送。 */
	UPROPERTY(Config)
	bool bEnableSessionsBasedPresence = false;

	/** 游戏中状态在后端使用的状态文本。 */
	UPROPERTY(Config)
	FString PresenceStatusInGame;

	/** 主菜单状态在后端使用的状态文本。 */
	UPROPERTY(Config)
	FString PresenceStatusMainMenu;

	/** 匹配中状态在后端使用的状态文本。 */
	UPROPERTY(Config)
	FString PresenceStatusMatchmaking;

	/** 保存游戏模式名称的在线状态属性键。 */
	UPROPERTY(Config)
	FString PresenceKeyGameMode;

	/** 保存地图名称的在线状态属性键。 */
	UPROPERTY(Config)
	FString PresenceKeyMapName;
};

#undef UE_API
