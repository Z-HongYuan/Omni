// Copyright © 2026 张鸿源. All Rights Reserved.

#include "Modules/ModuleManager.h"

#include "Cheats/ContraCheats.h"
#include "GameFramework/CheatManagerDefines.h"

class FOmniContraRuntimeModule : public FDefaultModuleImpl
{
public:
	virtual void StartupModule() override
	{
#if UE_WITH_CHEAT_MANAGER
		CheatManagerRegistrationHandle = UCheatManager::RegisterForOnCheatManagerCreated(
			FOnCheatManagerCreated::FDelegate::CreateLambda([](UCheatManager* CheatManager)
			{
				CheatManager->AddCheatManagerExtension(NewObject<UContraCheats>(CheatManager));
			}));
#endif
	}

	virtual void ShutdownModule() override
	{
#if UE_WITH_CHEAT_MANAGER
		UCheatManager::UnregisterFromOnCheatManagerCreated(CheatManagerRegistrationHandle);
#endif
	}

private:
	FDelegateHandle CheatManagerRegistrationHandle;
};

IMPLEMENT_MODULE(FOmniContraRuntimeModule, OmniContraRuntime)
