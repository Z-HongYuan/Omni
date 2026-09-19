// Copyright © 2026 张鸿源. All Rights Reserved.
#pragma once
#include "Import/OmniImportReporter.h"
class USkeletalMesh;
namespace Omni::CharacterImport
{
bool EnsurePostProcessAnimationBlueprint(USkeletalMesh* Mesh, const FImportReporter& Report);
}
