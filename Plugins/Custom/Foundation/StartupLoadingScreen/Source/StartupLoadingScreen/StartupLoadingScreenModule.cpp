// Copyright Epic Games, Inc. All Rights Reserved.

#include "StartupPreLoadScreen.h"
#include "Misc/App.h"
#include "Modules/ModuleManager.h"
#include "PreLoadScreenManager.h"

// 在引擎加载阶段显示可扩展的 Slate 界面，目前提供纯黑背景。
// 编辑器/PIE、专用服务器和无渲染环境中不创建界面；模块不在命令行工具中加载。
// 本插件与项目设置“影片”中的默认启动影片二选一；使用默认启动影片时应禁用本插件。
class FStartupLoadingScreenModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	virtual bool IsGameModule() const override;

private:
	void OnPreLoadScreenManagerCleanUp();

	TSharedPtr<FStartupPreLoadScreen> PreLoadingScreen;
	FDelegateHandle PreLoadScreenManagerCleanUpHandle;
};

void FStartupLoadingScreenModule::StartupModule()
{
	if (IsRunningDedicatedServer() || GIsEditor || !FApp::CanEverRender())
	{
		return;
	}

	FPreLoadScreenManager* Manager = FPreLoadScreenManager::Get();
	if (!Manager)
	{
		return;
	}

	PreLoadingScreen = MakeShared<FStartupPreLoadScreen>();
	PreLoadingScreen->Init();

	Manager->RegisterPreLoadScreen(PreLoadingScreen);
	PreLoadScreenManagerCleanUpHandle = Manager->OnPreLoadScreenManagerCleanUp.AddRaw(this, &FStartupLoadingScreenModule::OnPreLoadScreenManagerCleanUp);
}

void FStartupLoadingScreenModule::ShutdownModule()
{
	if (FPreLoadScreenManager* Manager = FPreLoadScreenManager::Get())
	{
		Manager->OnPreLoadScreenManagerCleanUp.Remove(PreLoadScreenManagerCleanUpHandle);

		if (PreLoadingScreen.IsValid())
		{
			// 只结束本模块正在显示的界面；等待渲染线程退出后才能注销并释放控件。
			if (PreLoadingScreen->IsPlaying())
			{
				PreLoadingScreen->SetEngineLoadingFinished(true);
				Manager->WaitForEngineLoadingScreenToFinish();
			}

			Manager->UnRegisterPreLoadScreen(PreLoadingScreen);
		}
	}
	else if (PreLoadingScreen.IsValid())
	{
		PreLoadingScreen->CleanUp();
	}

	PreLoadScreenManagerCleanUpHandle.Reset();
	PreLoadingScreen.Reset();
}

bool FStartupLoadingScreenModule::IsGameModule() const
{
	return true;
}

void FStartupLoadingScreenModule::OnPreLoadScreenManagerCleanUp()
{
	// 管理器已停止界面并调用 CleanUp；这里只解除绑定和引用，模块本身仍然有效。
	if (FPreLoadScreenManager* Manager = FPreLoadScreenManager::Get())
	{
		Manager->OnPreLoadScreenManagerCleanUp.Remove(PreLoadScreenManagerCleanUpHandle);
	}

	PreLoadScreenManagerCleanUpHandle.Reset();
	PreLoadingScreen.Reset();
}

IMPLEMENT_MODULE(FStartupLoadingScreenModule, StartupLoadingScreen)
