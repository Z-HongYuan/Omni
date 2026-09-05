// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Subsystems/WorldSubsystem.h"

#include "PocketLevelSystem.generated.h"

#define UE_API POCKETWORLDS_API

class ULocalPlayer;
class UObject;
class UPocketLevel;
class UPocketLevelInstance;

/**
 * 口袋世界的管理子系统，按本地玩家创建并复用流送实例。
 */
UCLASS(MinimalAPI)
class UPocketLevelSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// 获取（或首次创建）指定本地玩家在该口袋世界中的流送实例。
	UE_API UPocketLevelInstance* GetOrCreatePocketLevelFor(ULocalPlayer* LocalPlayer, UPocketLevel* PocketLevel, FVector DesiredSpawnPoint);

private:
	// 当前已创建的全部口袋世界实例。
	UPROPERTY()
	TArray<TObjectPtr<UPocketLevelInstance>> PocketInstances;
};

#undef UE_API
