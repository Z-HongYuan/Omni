// Copyright © 2026 张鸿源. All Rights Reserved.
#pragma once

#include "Import/OmniImportReporter.h"

class UMaterialInterface;
class UMaterialInstanceConstant;
class USkeletalMesh;
class UToonProfile;

namespace Omni::CharacterImport
{
UToonProfile* ResolveBodyToonProfileTemplate(UMaterialInterface* Parent, UToonProfile* ExplicitTemplate);
void SetOwnedToonProfile(UMaterialInstanceConstant* Material, UToonProfile* Profile);
void ClearOwnedToonProfile(UMaterialInstanceConstant* Material);
void ApplyBodyToonProfile(const TSet<UMaterialInstanceConstant*>& Targets, USkeletalMesh* Mesh,
	UToonProfile* Template, const FImportReporter& Report);
}
