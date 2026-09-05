// Copyright Epic Games, Inc. All Rights Reserved.

#include "PreLoadScreenManager.h"
#include "StartupPreLoadScreen.h"
#include "Misc/App.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FStartupLoadingScreenModule"

//  此模块创建一个 FStartupPreLoadScreen，它继承自 FPreLoadScreenBase
// 在引擎启动过程中，屏幕会显示一个动画小部件。
// 如果您想在引擎启动期间播放影片，则必须禁用本插件
// 这是因为在预加载期间，要么可以显示小部件，要么可以播放影片。
//  
// 您可以在项目设置 -> 影片中配置启动影片

class FStartupLoadingScreenModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	virtual bool IsGameModule() const override;

private:
	void OnPreLoadScreenManagerCleanUp();

	TSharedPtr<FStartupPreLoadScreen> PreLoadingScreen;
};

void FStartupLoadingScreenModule::StartupModule()
{
	// 无需在专用服务器上加载这些资源。
	// 但仍需在命令行工具中加载它们，以便cook过程能够捕获到它们。
	if (!IsRunningDedicatedServer())
	{
		PreLoadingScreen = MakeShared<FStartupPreLoadScreen>();
		PreLoadingScreen->Init();

		if (!GIsEditor && FApp::CanEverRender() && FPreLoadScreenManager::Get())
		{
			FPreLoadScreenManager::Get()->RegisterPreLoadScreen(PreLoadingScreen);
			FPreLoadScreenManager::Get()->OnPreLoadScreenManagerCleanUp.AddRaw(this, &FStartupLoadingScreenModule::OnPreLoadScreenManagerCleanUp);
		}
	}
}

void FStartupLoadingScreenModule::ShutdownModule()
{
}

bool FStartupLoadingScreenModule::IsGameModule() const
{
	return true;
}

void FStartupLoadingScreenModule::OnPreLoadScreenManagerCleanUp()
{
	// 模块关闭后,处理器和控件都可以开始销毁了
	PreLoadingScreen.Reset();
	ShutdownModule();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FStartupLoadingScreenModule, StartupLoadingScreen)
