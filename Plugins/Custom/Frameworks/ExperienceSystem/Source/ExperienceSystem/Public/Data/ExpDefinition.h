// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Engine/DataAsset.h"
#include "ExpDefinition.generated.h"

#define UE_API EXPERIENCESYSTEM_API

class UExpActionSet;
class UGameFeatureAction;
class UExpPawnData;

/** 体验定义资产，组合默认 PawnData、Game Feature 插件和 Action 集合。 */
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
	/** 需要加载并激活的 Game Feature 插件名称。 */
	UPROPERTY(EditDefaultsOnly, Category=Gameplay)
	TArray<FString> GameFeaturesToEnable;

	/** 玩家默认 PawnData；TODO: 评估是否需要改为软引用。 */
	UPROPERTY(EditDefaultsOnly, Category=Gameplay)
	TObjectPtr<const UExpPawnData> DefaultPawnData;

	/** 随体验注册、加载、激活、停用和注销的 Action 实例。 */
	UPROPERTY(EditDefaultsOnly, Instanced, Category=Actions)
	TArray<TObjectPtr<UGameFeatureAction>> Actions;

	/** 与本体验共同加载和执行的共享 Action 集合。 */
	UPROPERTY(EditDefaultsOnly, Category=Gameplay)
	TArray<TObjectPtr<UExpActionSet>> ActionSets;
};
#undef UE_API
