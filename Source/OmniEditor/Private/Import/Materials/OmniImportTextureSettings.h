// Copyright © 2026 张鸿源. All Rights Reserved.
#pragma once

#include "Import/Types/OmniKeywordMaterialBinding.h"
#include "Import/OmniImportReporter.h"

class UInterchangeBaseNodeContainer;

namespace Omni::CharacterImport
{
// Only new source textures are configured; existing project textures and reimports are left untouched.
bool PrepareTextureSettings(UInterchangeBaseNodeContainer* Container,
	const TArray<FOmniKeywordMaterialBinding>& Materials, const FImportReporter& Report);
}
