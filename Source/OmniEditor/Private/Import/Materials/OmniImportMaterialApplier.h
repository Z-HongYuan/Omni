// Copyright © 2026 张鸿源. All Rights Reserved.
#pragma once

#include "Import/Types/OmniFaceTextureBinding.h"
#include "Import/OmniImportReporter.h"

class UInterchangeBaseNodeContainer;
class UMaterialInterface;
class UMaterialInstanceConstant;
class UTexture;

namespace Omni::CharacterImport
{
bool SetTextureParameterVerified(UMaterialInstanceConstant* Material, FName Parameter, UTexture* Texture);
bool PrepareMaterialTextures(UInterchangeBaseNodeContainer* Container, const FString& FactoryUid,
	const TSoftObjectPtr<UMaterialInterface>& Parent, const TArray<FOmniFaceTextureBinding>& Bindings, const FImportReporter& Report);
void ApplyMaterialTextures(const UInterchangeBaseNodeContainer* Container, const FString& NodeKey,
	UMaterialInstanceConstant* Material, UMaterialInterface* Parent, const TArray<FOmniFaceTextureBinding>& Bindings, const FImportReporter& Report);
}
