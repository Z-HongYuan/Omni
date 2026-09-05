// Copyright 2024 bstt, Inc. All Rights Reserved.

#include "../Public/AutoNodeArrangerModule.h"
#include "./Commands/ANA_Commands.h"
#include "./Config/ANA_Settings.h"
#include "./Helpers/ANA_InputProcessor.h"
#include "./Helpers/ANA_ReleaseNotifier.h"
#include "./Helpers/Arranger/ANA_NodeRegister.h"

void FAutoNodeArrangerModule::StartupModule()
{
	ANA_Commands::Register();
	ANA_Commands::BindAllCommands();
	ANA_Settings::RegisterSettings();
	ANA_InputProcessor::RegisterProcessor();
	ANA_ReleaseNotifier::Get().RegisterCheckVersion();
	ANA_NodeRegister::Get().StartAutoRegister();
}

void FAutoNodeArrangerModule::ShutdownModule()
{
	ANA_NodeRegister::Get().StopAutoRegister();
	ANA_InputProcessor::UnregisterProcessor();
	ANA_Settings::UnregisterSettings();
	ANA_Commands::UnbindAllCommands();
	ANA_Commands::Unregister();
}

IMPLEMENT_MODULE(FAutoNodeArrangerModule, AutoNodeArranger)