// Copyright © 2026 张鸿源. All Rights Reserved.
#pragma once

#include "Engine/AssetUserData.h"
#include "OmniImportMaterialUserData.generated.h"

class UToonProfile;

/** 仅供编辑器使用的所有权记录，UObject 引用随资产移动和重命名更新。 */
UCLASS()
class UOmniImportMaterialUserData final : public UAssetUserData
{
	GENERATED_BODY()

public:
	virtual bool IsEditorOnly() const override { return true; }

	UPROPERTY()
	TObjectPtr<UToonProfile> OwnedToonProfile;

	/** 导入组预定的包路径，不随 Profile 当前位置改变；空值表示旧版单材质实例所有权。 */
	UPROPERTY()
	FString SharedProfileGroupKey;
};
