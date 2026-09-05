// Copyright © 2026 张鸿源. All Rights Reserved.


#include "System/OmniEditorEngine.h"

#include "OmniEditor/OmniEditorLogChannel.h"
#include "Settings/ContentBrowserSettings.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(OmniEditorEngine)

void UOmniEditorEngine::Init(IEngineLoop* InEngineLoop)
{
	Super::Init(InEngineLoop);

	UE_LOG(LogOmniEditor, Log, TEXT("OmniEditorEngine Init"));

	// 加载时强制显示插件内容。
	GetMutableDefault<UContentBrowserSettings>()->SetDisplayPluginFolders(true);
}
