// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "CosmeticHelper.generated.h"

#define UE_API CUSTOMCOSMETICS_API

/**
 * 用于外观系统的静态函数
 */
UCLASS(MinimalAPI)
class UCosmeticHelper : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// 尝试找一个短名称的类（通过作弊控制台时会有一些启发式以提高可用性）
	static UClass* FindClassByShortName(const FString& SearchToken, UClass* DesiredBaseClass, bool bLogFailures = true);

	template <typename DesiredClass>
	static TSubclassOf<DesiredClass> FindClassByShortName(const FString& SearchToken, bool bLogFailures = true)
	{
		return FindClassByShortName(SearchToken, DesiredClass::StaticClass(), bLogFailures);
	}

private:
	static TArray<FAssetData> GetAllBlueprints();
	static UClass* FindBlueprintClass(const FString& TargetNameRaw, UClass* DesiredBaseClass);
};
#undef UE_API
