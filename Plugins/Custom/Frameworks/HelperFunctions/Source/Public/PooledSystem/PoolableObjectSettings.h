// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Engine/DeveloperSettings.h"
#include "UObject/SoftObjectPtr.h"
#include "PoolableObjectSettings.generated.h"

#define UE_API HELPERFUNCTIONS_API

/** 配置各对象类型在池中的默认实例数量上限。 */
UCLASS(MinimalAPI, Config = Game, Defaultconfig)
class UPoolableObjectSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/** 返回 Project Settings 中的设置类别。 */
	UE_API virtual FName GetCategoryName() const override { return TEXT("ZHYS"); }

	UE_API virtual FText GetSectionDescription() const override;

	/** 各类型的实例数量上限；未配置或值不大于零时不限制数量。 */
	UPROPERTY(EditAnywhere, Config, Category = "PoolableObject")
	TMap<TSoftClassPtr<UObject>, int32> DefaultMaxSizes;
};
#undef UE_API
