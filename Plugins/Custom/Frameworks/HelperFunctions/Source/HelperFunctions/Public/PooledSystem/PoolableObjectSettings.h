// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Engine/DeveloperSettings.h"
#include "UObject/SoftObjectPtr.h"
#include "PoolableObjectSettings.generated.h"

#define UE_API HELPERFUNCTIONS_API

/**
 * 简要对象池的配置
 */
UCLASS(MinimalAPI, Config = Game, Defaultconfig)
class UPoolableObjectSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/** 返回设置类别名称（在 Project Settings 中的路径） */
	UE_API virtual FName GetCategoryName() const override { return TEXT("ZHYS"); }

	UE_API virtual FText GetSectionDescription() const override;

	//每个类的最大数量,未设置则为无限
	UPROPERTY(EditAnywhere, Config, Category = "PoolableObject")
	TMap<TSoftClassPtr<UObject>, int32> DefaultMaxSizes;
};
#undef UE_API
