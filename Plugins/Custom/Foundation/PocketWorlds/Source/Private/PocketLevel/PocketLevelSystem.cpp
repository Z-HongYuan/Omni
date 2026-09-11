// Copyright Epic Games, Inc. All Rights Reserved.

#include "PocketLevel/PocketLevelSystem.h"

#include "Engine/LocalPlayer.h"
#include "PocketLevel/PocketLevel.h"
#include "PocketLevel/PocketLevelInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PocketLevelSystem)

UPocketLevelInstance* UPocketLevelSubsystem::GetOrCreatePocketLevelFor(ULocalPlayer* LocalPlayer, UPocketLevel* PocketLevel, FVector DesiredSpawnPoint)
{
	if (!IsValid(LocalPlayer) || !IsValid(PocketLevel) || !IsValid(GetWorld()) || LocalPlayer->GetWorld() != GetWorld())
	{
		return nullptr;
	}

	float VerticalBoundsOffset = 0;
	for (UPocketLevelInstance* Instance : PocketInstances)
	{
		if (Instance->LocalPlayer == LocalPlayer && Instance->PocketLevel == PocketLevel)
		{
			return Instance;
		}

		VerticalBoundsOffset += Instance->PocketLevel->BoundsSize.Z;
	}

	const FVector SpawnPoint = DesiredSpawnPoint + FVector(0, 0, VerticalBoundsOffset);

	UPocketLevelInstance* NewInstance = NewObject<UPocketLevelInstance>(this);
	// 初始化失败时不返回或缓存实例，后续请求仍可重新尝试。
	if (!NewInstance->Initialize(LocalPlayer, PocketLevel, SpawnPoint))
	{
		return nullptr;
	}

	PocketInstances.Add(NewInstance);

	return NewInstance;
}
