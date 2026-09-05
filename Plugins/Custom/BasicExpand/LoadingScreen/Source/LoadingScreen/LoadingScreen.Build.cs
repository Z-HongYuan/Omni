// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class LoadingScreen : ModuleRules
{
	public LoadingScreen(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"UMG"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"DeveloperSettings",
				"PreLoadScreen",
				"Slate",
				"SlateCore",
				"RenderCore",
			}
		);
	}
}