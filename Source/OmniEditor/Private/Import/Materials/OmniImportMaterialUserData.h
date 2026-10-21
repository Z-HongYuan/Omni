// Copyright © 2026 张鸿源. All Rights Reserved.
#pragma once

#include "Engine/AssetUserData.h"
#include "OmniImportMaterialUserData.generated.h"

class UToonProfile;

/** Editor-only ownership record; UObject references follow asset moves and renames. */
UCLASS()
class UOmniImportMaterialUserData final : public UAssetUserData
{
	GENERATED_BODY()

public:
	virtual bool IsEditorOnly() const override { return true; }

	UPROPERTY()
	TObjectPtr<UToonProfile> OwnedToonProfile;

	/** Intended group package, independent of the Profile's current path. Empty marks legacy per-MI ownership. */
	UPROPERTY()
	FString SharedProfileGroupKey;
};
