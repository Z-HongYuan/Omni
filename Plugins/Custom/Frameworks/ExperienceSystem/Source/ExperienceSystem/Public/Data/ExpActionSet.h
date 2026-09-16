// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Engine/DataAsset.h"
#include "ExpActionSet.generated.h"

#define UE_API EXPERIENCESYSTEM_API

class UGameFeatureAction;

/**
 * 在体验系统中定义的共享执行合集
 */
UCLASS(BlueprintType, NotBlueprintable, MinimalAPI)
class UExpActionSet : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	//~UObject interface
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
	//~End of UObject interface

	//~UPrimaryDataAsset interface
#if WITH_EDITORONLY_DATA
	virtual void UpdateAssetBundleData() override;
#endif
	//~End of UPrimaryDataAsset interface

public:
	// 加载/激活/停用/卸载 时要执行的操作列表
	UPROPERTY(EditAnywhere, Instanced, Category="Actions")
	TArray<TObjectPtr<UGameFeatureAction>> Actions;

	// 将会在体验系统中加载的 Game Feature Plugins 路径列表
	UPROPERTY(EditAnywhere, Category="Features")
	TArray<FString> GameFeaturesToEnable;
};
#undef UE_API
