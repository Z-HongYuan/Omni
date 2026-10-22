// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine/DataAsset.h"

#include "PocketLevel.generated.h"

#define UE_API POCKETWORLDS_API

class UObject;
class UWorld;

/** 描述可动态流送的口袋关卡及实例间的位置偏移范围。 */
UCLASS(MinimalAPI)
class UPocketLevel : public UDataAsset
{
	GENERATED_BODY()

public:
	/** 该口袋世界需要流送进来的关卡。 */
	UPROPERTY(EditAnywhere, Category=Pocket)
	TSoftObjectPtr<UWorld> SubLevel;

	/** 口袋关卡的尺寸参数，Z 分量用于累加多个实例的纵向生成偏移。 */
	UPROPERTY(EditAnywhere, Category=Pocket)
	FVector BoundsSize;
};

#undef UE_API
