// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Logging/LogCategory.h"

//定义Log
DECLARE_LOG_CATEGORY_EXTERN(LogGameCoreExtension, Log, All);

GAMECOREEXTENSION_API FString GetClientServerContextString(UObject* ContextObject = nullptr);
