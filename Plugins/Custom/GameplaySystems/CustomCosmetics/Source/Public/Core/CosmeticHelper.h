// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "CosmeticHelper.generated.h"

#define UE_API CUSTOMCOSMETICS_API

/** 为外观调试命令提供类名和蓝图资产查询。 */
UCLASS(MinimalAPI)
class UCosmeticHelper : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** 按短名称或对象路径查找指定基类的派生类；必要时查询并加载蓝图资产。 */
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
