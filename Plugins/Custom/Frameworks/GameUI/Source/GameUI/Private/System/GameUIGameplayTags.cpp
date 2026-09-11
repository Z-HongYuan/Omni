// Copyright © 2026 张鸿源. All Rights Reserved.


#include "System/GameUIGameplayTags.h"

#define UE_API GAMEUI_API

namespace GameUITags
{
	UE_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GameUI_UIStack_Modal, "AdvancedUI.UIStack.Modal", "UI系统: 模态栈,用于弹窗之类,位于层次的最顶部")
	UE_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GameUI_UIStack_GameMenu, "AdvancedUI.UIStack.GameMenu", "UI系统: 菜单栈,用于暂停菜单之类,位于第二层")
	UE_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GameUI_UIStack_GameHUD, "AdvancedUI.UIStack.GameHUD", "UI系统: HUD栈,用于游戏界面,血条,蓝条之类,位于第三层")
	UE_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GameUI_UIStack_Frontend, "AdvancedUI.UIStack.Frontend", "UI系统: UI前端栈,用于UI界面,位于最底层")

	UE_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GameUI_Dialog_Confirmed, "AdvancedUI.Dialog.Confirmed", "对话系统: 按下了“是”按钮")
	UE_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GameUI_Dialog_Declined, "AdvancedUI.Dialog.Declined", "对话系统: 按下了“不”按钮")
	UE_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GameUI_Dialog_Cancelled, "AdvancedUI.Dialog.Cancelled", "对话系统: 按下了“忽略取消”按钮")
	UE_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GameUI_Dialog_Killed, "AdvancedUI.Dialog.Killed", "对话系统: 对话被明确关闭（无用户输入）")
}

#undef UE_API
