// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "CosmeticsControllerComponent.h"
#include "GameFramework/CheatManager.h"
#include "CosmeticCheats.generated.h"

#define UE_API CUSTOMCOSMETICS_API

/** 外观调试命令扩展，在启用 CheatManager 的构建中自动注册。 */
UCLASS(MinimalAPI, NotBlueprintable)
class UCosmeticCheats : public UCheatManagerExtension
{
	GENERATED_BODY()

public:
	UCosmeticCheats();

	/** 添加调试外观，可同时隐藏常规外观部件。 */
	UFUNCTION(Exec, BlueprintAuthorityOnly)
	UE_API void AddCharacterPart(const FString& AssetName, bool bSuppressNaturalParts = true);

	/** 清除现有命令覆盖后，添加指定的调试外观。 */
	UFUNCTION(Exec, BlueprintAuthorityOnly)
	UE_API void ReplaceCharacterPart(const FString& AssetName, bool bSuppressNaturalParts = true);

	/** 清除命令添加的调试外观，并重新应用开发者设置。 */
	UFUNCTION(Exec, BlueprintAuthorityOnly)
	UE_API void ClearCharacterPartOverrides();

private:
	UCosmeticsControllerComponent* GetCosmeticComponent() const;
};

#undef UE_API
