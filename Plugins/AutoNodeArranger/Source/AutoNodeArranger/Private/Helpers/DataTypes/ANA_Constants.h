// Copyright 2024 bstt, Inc. All Rights Reserved.

#pragma once

#include "Interfaces/IPluginManager.h"
#include "Modules/ModuleManager.h"

namespace ANA_Constants
{
	static const FString PluginBaseDirFullPath
		= FPaths::ConvertRelativePathToFull(IPluginManager::Get().FindPlugin("AutoNodeArranger")->GetBaseDir());
	static const bool isAutoSizeCommentLoaded = FModuleManager::Get().IsModuleLoaded("AutoSizeComments");
}; // namespace ANA_Constants