// Copyright © 2026 张鸿源. All Rights Reserved.
#pragma once

#include "Import/OmniImportReporter.h"

class UMaterialInterface;
class UMaterialInstanceConstant;
class UToonProfile;

namespace Omni::CharacterImport
{
UToonProfile* ResolveBodyToonProfileTemplate(UMaterialInterface* Parent, UToonProfile* ExplicitTemplate);
void SetOwnedToonProfile(UMaterialInstanceConstant* Material, UToonProfile* Profile, const FString& GroupKey = FString());
void ClearOwnedToonProfile(UMaterialInstanceConstant* Material);
/** Reuses a uniquely owned group profile (including moves/renames), then the canonical package, or copies the template. */
UToonProfile* ResolveOrCreateSharedToonProfile(const FString& Package, const TArray<UMaterialInstanceConstant*>& ExistingMIs,
	UToonProfile* Template, const FImportReporter& Report);
/** Assigns the resolved group profile without creating assets; a null profile removes only an owned override. */
void ApplySharedToonProfile(UMaterialInstanceConstant* Material, UToonProfile* SharedProfile, const FString& GroupKey,
	const FImportReporter& Report);
}
