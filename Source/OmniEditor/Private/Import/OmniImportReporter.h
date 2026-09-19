// Copyright © 2026 张鸿源. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"

namespace Omni::CharacterImport
{
struct FImportReporter
{
	TFunction<void(const FString&, bool)> Write;
	void operator()(const FString& Message, bool bError = false) const { Write(Message, bError); }
};
}
