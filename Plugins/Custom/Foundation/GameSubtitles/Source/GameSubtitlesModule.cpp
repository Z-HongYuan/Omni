// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameplayTagsManager.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"

class FGameSubtitlesModule : public IModuleInterface
{
public:
	/** IModuleInterface 接口实现 */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

void FGameSubtitlesModule::StartupModule()
{
	// 按实际安装位置查找配置，支持插件放在自定义子目录或引擎插件目录。
	if (const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("GameSubtitles"));
		ensureMsgf(Plugin.IsValid(), TEXT("未找到 GameSubtitles 插件，无法加载字幕标签配置。")))
	{
		UGameplayTagsManager::Get().AddTagIniSearchPath(FPaths::Combine(Plugin->GetBaseDir(), TEXT("Config/Tags")));
	}
}

void FGameSubtitlesModule::ShutdownModule()
{
}

IMPLEMENT_MODULE(FGameSubtitlesModule, GameSubtitles)
