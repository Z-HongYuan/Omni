// Copyright © 2026 张鸿源. All Rights Reserved.

#include "Editor.h"
#include "Engine/Engine.h"
#include "Gameplay/ExpPluginCountManager.h"
#include "Modules/ModuleManager.h"

/** 转发 BeginPIE 事件以重置插件请求计数，并在模块退出时解除监听。 */
class FExperienceSystemEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override
	{
		if (!IsRunningGame())
		{
			FEditorDelegates::BeginPIE.AddRaw(this, &FExperienceSystemEditorModule::OnBeginPIE);
		}
	}

	virtual void ShutdownModule() override
	{
		FEditorDelegates::BeginPIE.RemoveAll(this);
	}

private:
	void OnBeginPIE(bool bIsSimulating)
	{
		UExpPluginCountManager* PluginCountManager = GEngine->GetEngineSubsystem<UExpPluginCountManager>();
		check(PluginCountManager);
		PluginCountManager->OnPlayInEditorBegun();
	}
};

IMPLEMENT_MODULE(FExperienceSystemEditorModule, ExperienceSystemEditor)
