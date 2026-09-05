// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class AdvancedUI : ModuleRules
{
	public AdvancedUI(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"UMG", "CommonUI",
				"InputCore", "CommonInput",
				"GameplayTags",
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"DeveloperSettings",
				"Slate",
				"SlateCore",
			}
		);
	}
}