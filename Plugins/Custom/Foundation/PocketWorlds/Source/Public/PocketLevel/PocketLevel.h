// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine/DataAsset.h"

#include "PocketLevel.generated.h"

#define UE_API POCKETWORLDS_API

class UObject;
class UWorld;

/**
 * 口袋世界的数据资产：描述一个可被流送进来的小关卡及其尺寸。
 */
UCLASS(MinimalAPI)
class UPocketLevel : public UDataAsset
{
	GENERATED_BODY()

public:
	// 该口袋世界需要流送进来的关卡。
	UPROPERTY(EditAnywhere, Category=Pocket)
	TSoftObjectPtr<UWorld> SubLevel;

	// 口袋世界的尺寸范围，用于在创建多个实例时彼此错开、避免重叠。
	UPROPERTY(EditAnywhere, Category=Pocket)
	FVector BoundsSize;
};

#undef UE_API
