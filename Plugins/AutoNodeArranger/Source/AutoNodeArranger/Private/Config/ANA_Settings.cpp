// Copyright 2024 bstt, Inc. All Rights Reserved.

#pragma once

#include "ANA_Settings.h"
#include "../Copy_ApplicationRestartRequiredNotification.h"
#include "ANA_EditorConfig.h"
#include "Helpers/ANA_ReleaseNotifier.h"
#include "ISettingsModule.h"
#include "ISettingsSection.h"
#include "Modules/ModuleManager.h"

void ANA_Settings::RegisterSettings()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
#define LOCTEXT_NAMESPACE "FAutoNodeArrangerModule"
		ISettingsSectionPtr EditorSettingsSection = SettingsModule->RegisterSettings("Editor",
			"Plugins",
			"Auto Node Arranger",
			LOCTEXT("RuntimeGeneralSettingsName", "Auto Node Arranger"),
			LOCTEXT("RuntimeGeneralSettingsDescription", "Editor configuration for Auto Node Arranger module"),
			UANA_EditorConfig::Get());
#undef LOCTEXT_NAMESPACE
		static FCopy_ApplicationRestartRequiredNotification ApplicationRestartRequiredNotification;
		EditorSettingsSection->OnModified().BindLambda(
			[]()
			{
				if (UANA_EditorConfig::CheckShowNewFeatures())
					ANA_ReleaseNotifier::Get().ShowNotification(EANA_ReleaseStep::AUTO_ARRANGE_THANKS);
				UANA_EditorConfig::Get()->SaveConfig();
				return true;
			});
		EditorSettingsSection->OnResetDefaults().BindLambda(
			[EditorSettingsSection]()
			{
				resetConfigToDefaultValues(UANA_EditorConfig::Get());
				ApplicationRestartRequiredNotification.OnRestartRequired();
				return true;
			});
	}
}

void ANA_Settings::UnregisterSettings()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings("Editor", "Plugins", "Auto Node Arranger");
		SettingsModule->UnregisterSettings("Project", "Plugins", "Auto Node Arranger");
	}
}

void ANA_Settings::resetConfigToDefaultValues(UObject* SettingsObject)
{
	FString ConfigName = SettingsObject->GetClass()->GetConfigName();

	GConfig->EmptySection(*SettingsObject->GetClass()->GetPathName(), ConfigName);
	GConfig->Flush(false);

	FConfigCacheIni::LoadGlobalIniFile(ConfigName, *FPaths::GetBaseFilename(ConfigName), nullptr, true);

	SettingsObject->ReloadConfig(nullptr, nullptr, UE::LCPF_PropagateToInstances | UE::LCPF_PropagateToChildDefaultObjects);
}