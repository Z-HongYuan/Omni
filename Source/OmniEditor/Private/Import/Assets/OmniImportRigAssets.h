// Copyright © 2026 张鸿源. All Rights Reserved.
#pragma once
#include "Import/OmniImportReporter.h"
class USkeletalMesh;
class UOmniCharacterImportProfile;
namespace Omni::CharacterImport
{
void CreateRigAssets(USkeletalMesh* Mesh, const UOmniCharacterImportProfile& Profile,
	const FString& CharacterRoot, const FString& ResolvedName, const FImportReporter& Report);
}
