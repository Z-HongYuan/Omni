// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "CosmeticsControllerComponent.h"
#include "GameFramework/CheatManager.h"
#include "CosmeticCheats.generated.h"

#define UE_API CUSTOMCOSMETICS_API

/**
 * 用于外观装扮的作弊函数,自动添加到CheatManager中
 */
UCLASS(MinimalAPI, NotBlueprintable)
class UCosmeticCheats : public UCheatManagerExtension
{
	GENERATED_BODY()

public:
	UCosmeticCheats();

	// 添加一个外观
	UFUNCTION(Exec, BlueprintAuthorityOnly)
	UE_API void AddCharacterPart(const FString& AssetName, bool bSuppressNaturalParts = true);

	// 替换一个外观
	UFUNCTION(Exec, BlueprintAuthorityOnly)
	UE_API void ReplaceCharacterPart(const FString& AssetName, bool bSuppressNaturalParts = true);

	// 清除所有的外观
	UFUNCTION(Exec, BlueprintAuthorityOnly)
	UE_API void ClearCharacterPartOverrides();

private:
	UCosmeticsControllerComponent* GetCosmeticComponent() const;
};

#undef UE_API
