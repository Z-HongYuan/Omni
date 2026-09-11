// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class StartupLoadingScreen : ModuleRules
{
	public StartupLoadingScreen(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateDependencyModuleNames.AddRange(
			new[]
			{
				"Core",
				"CoreUObject",
				"PreLoadScreen",
				"SlateCore",
				"Slate"
			}
		);
	}
}