// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Engine/DataAsset.h"
#include "ExpDefinition.generated.h"

#define UE_API EXPERIENCESYSTEM_API

class UExpActionSet;
class UGameFeatureAction;
class UExpPawnData;

/**
 * 在体验系统中定义的体验资产
 * 包含各种必要配置信息
 * 例如 PawnData 和 要激活的 Game Feature Plugins
 */
UCLASS(MinimalAPI, BlueprintType, Const)
class UExpDefinition : public UPrimaryDataAsset
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
	// 需要激活的游戏功能插件列表
	UPROPERTY(EditDefaultsOnly, Category=Gameplay)
	TArray<FString> GameFeaturesToEnable;

	// 玩家使用的默认 PawnData 数据
	//@TODO: 是否需要软引用?
	UPROPERTY(EditDefaultsOnly, Category=Gameplay)
	TObjectPtr<const UExpPawnData> DefaultPawnData;

	// 加载/激活/停用/卸载 时要执行的操作列表
	UPROPERTY(EditDefaultsOnly, Instanced, Category=Actions)
	TArray<TObjectPtr<UGameFeatureAction>> Actions;

	// 额外执行的共享集
	UPROPERTY(EditDefaultsOnly, Category=Gameplay)
	TArray<TObjectPtr<UExpActionSet>> ActionSets;
};
#undef UE_API
