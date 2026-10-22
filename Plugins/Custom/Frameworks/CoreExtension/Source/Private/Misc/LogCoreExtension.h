// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "Logging/LogCategory.h"

// CoreExtension 日志分类。
DECLARE_LOG_CATEGORY_EXTERN(LogCoreExtension, Log, All);

COREEXTENSION_API FString GetClientServerContextString(UObject* ContextObject = nullptr);
