// Copyright © 2026 张鸿源. All Rights Reserved.

#include "Data/GameUserTypes.h"
#include "OnlineError.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameUserTypes)

void FOnlineResultInformation::FromOnlineError(const FOnlineErrorType& InOnlineError)
{
	bWasSuccessful = InOnlineError.WasSuccessful();
	ErrorId = InOnlineError.GetErrorCode();
	ErrorText = InOnlineError.GetErrorMessage();
}

namespace FGameUserTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_SystemMessage_Error, "SystemMessage.Error", "系统消息::错误")
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_SystemMessage_Warning, "SystemMessage.Warning", "系统消息::警告")
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_SystemMessage_Display, "SystemMessage.Display", "系统消息::显示")
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_SystemMessage_Error_InitializeLocalPlayerFailed, "SystemMessage.Error.InitializeLocalPlayerFailed", "系统消息::初始化本地玩家失败")

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Platform_Trait_RequiresStrictControllerMapping, "Platform.Trait.RequiresStrictControllerMapping", "平台特征::严格严格映射")
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Platform_Trait_SingleOnlineUser, "Platform.Trait.SingleOnlineUser", "平台特征::单用户在线")
}
