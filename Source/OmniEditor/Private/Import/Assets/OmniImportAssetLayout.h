// Copyright © 2026 张鸿源. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
class UInterchangeBaseNodeContainer;
class UOmniCharacterImportProfile;
namespace Omni::CharacterImport
{
bool IsRelativeFolder(const FString& Folder);
void ApplyAssetLayout(UInterchangeBaseNodeContainer* Container, const UOmniCharacterImportProfile& Profile, const FString& ResolvedName);
}
