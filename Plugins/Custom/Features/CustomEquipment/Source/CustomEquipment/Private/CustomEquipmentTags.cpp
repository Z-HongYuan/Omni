// Copyright © 2026 张鸿源. All Rights Reserved.


#include "CustomEquipmentTags.h"

#define UE_API CUSTOMEQUIPMENT_API

namespace CustomEquipmentTags
{
	UE_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_QuickBar_Message_SlotsChanged, "QuickBar.Message.SlotsChanged", "装备系统: 快捷栏消息.槽位已改变")
	UE_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_QuickBar_Message_ActiveIndexChanged, "QuickBar.Message.ActiveIndexChanged", "装备系统: 快捷栏消息.激活项目已改变")
}
#undef UE_API
