// Copyright © 2026 张鸿源. All Rights Reserved.
#pragma once

#include "Import/Types/OmniKeywordMaterialBinding.h"
#include "Import/OmniImportReporter.h"

class UInterchangeBaseNodeContainer;

namespace Omni::CharacterImport
{
/** 配置源纹理工厂的色彩空间与压缩；调用方仅在首次导入时使用，不修改直接引用的项目纹理。 */
bool PrepareTextureSettings(UInterchangeBaseNodeContainer* Container,
	const TArray<FOmniKeywordMaterialBinding>& Materials, const FImportReporter& Report);
}
