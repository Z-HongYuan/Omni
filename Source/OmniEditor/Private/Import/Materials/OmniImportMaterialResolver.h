// Copyright © 2026 张鸿源. All Rights Reserved.
#pragma once

#include "Import/Types/OmniFaceTextureBinding.h"

class UInterchangeBaseNode;
class UInterchangeBaseNodeContainer;
class UMaterialInterface;

namespace Omni::CharacterImport
{
TArray<UInterchangeBaseNode*> GetSourceMaterials(const UInterchangeBaseNodeContainer* Container);
void RefreshMaterialTextureParameters(UMaterialInterface* Parent, TArray<FOmniFaceTextureBinding>& Bindings);
void ResolveMaterialTextures(const UInterchangeBaseNodeContainer* Container, const UInterchangeBaseNode* SourceMaterial,
	const FString& RoleName, TArray<FOmniFaceTextureBinding>& Bindings, FString& OutError);
}
