// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Engine/DataAsset.h"
#include "ExpActionSet.generated.h"

#define UE_API EXPERIENCESYSTEM_API

class UGameFeatureAction;

/** 供多个体验复用的 Action 集合及插件依赖。 */
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
	/** 随体验注册、加载、激活、停用和注销的 Action 实例。 */
	UPROPERTY(EditAnywhere, Instanced, Category="Actions")
	TArray<TObjectPtr<UGameFeatureAction>> Actions;

	/** 需要加载并激活的 Game Feature 插件名称，由子系统解析为 URL。 */
	UPROPERTY(EditAnywhere, Category="Features")
	TArray<FString> GameFeaturesToEnable;
};
#undef UE_API
