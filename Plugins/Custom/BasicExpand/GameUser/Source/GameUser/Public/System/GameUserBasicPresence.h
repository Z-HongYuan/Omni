// Copyright © 2026 张鸿源. All Rights Reserved.
#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "GameUserBasicPresence.generated.h"

#define UE_API GAMEUSER_API

class UGameUserSessionSubsystem;
enum class EGameUserSessionInformationState : uint8;

/**
 * 该子系统接入会话子系统,并将其信息推送到在线状态接口。
 * 它并非要作为功能完整的富在线状态实现,而是可以用作概念验证,
 * 用于将会话子系统的信息推送到在线状态系统
 */
UCLASS(MinimalAPI, BlueprintType, Config = Engine)
class UGameUserBasicPresence : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UE_API virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	UE_API virtual void Deinitialize() override;

	UE_API void OnNotifySessionInformationChanged(EGameUserSessionInformationState SessionStatus, const FString& GameMode, const FString& MapName);
	UE_API FString SessionStateToBackendKey(EGameUserSessionInformationState SessionStatus);

	/** 设为 False 是一个通用开关,用于阻止此类推送在线状态*/
	UPROPERTY(Config)
	bool bEnableSessionsBasedPresence = false;

	/** 将在线状态"In-game"映射到后端键*/
	UPROPERTY(Config)
	FString PresenceStatusInGame;

	/** 将在线状态"Main Menu"映射到后端键*/
	UPROPERTY(Config)
	FString PresenceStatusMainMenu;

	/** 将在线状态"Matchmaking"映射到后端键*/
	UPROPERTY(Config)
	FString PresenceStatusMatchmaking;

	/** 将"Game Mode"富在线状态条目映射到后端键*/
	UPROPERTY(Config)
	FString PresenceKeyGameMode;

	/** 将"Map Name"富在线状态条目映射到后端键*/
	UPROPERTY(Config)
	FString PresenceKeyMapName;
};

#undef UE_API
