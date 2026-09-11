// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "NativeGameplayTags.h"

#define UE_API GAMEUI_API

namespace AdvancedUITags
{
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_AdvancedUI_UIStack_Modal);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_AdvancedUI_UIStack_GameMenu);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_AdvancedUI_UIStack_GameHUD);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_AdvancedUI_UIStack_Frontend);

	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_AdvancedUI_Dialog_Confirmed);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_AdvancedUI_Dialog_Declined);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_AdvancedUI_Dialog_Cancelled);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_AdvancedUI_Dialog_Killed);
}

#undef UE_API
