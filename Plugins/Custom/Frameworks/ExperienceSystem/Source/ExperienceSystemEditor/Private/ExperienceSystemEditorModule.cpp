// Copyright © 2026 张鸿源. All Rights Reserved.

#include "Editor.h"
#include "Engine/Engine.h"
#include "Gameplay/ExpPluginCountManager.h"
#include "Modules/ModuleManager.h"

// 在BeginPlay的时候重置插件计数器
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
