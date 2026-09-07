// Copyright © 2026 张鸿源. All Rights Reserved.

#pragma once

#include "NativeGameplayTags.h"

/**
 * 整个项目使用的GameplayTags基础定义
 */
#define UE_API OMNIGAME_API

namespace OmniTags
{
	//~基础原生输入动作（InputConfig 里配置 InputAction 时按这些 Tag 对应，绑定到输入组件的处理函数）
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_InputTag_Move);
	UE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_InputTag_Look_Mouse);
}

#undef UE_API
