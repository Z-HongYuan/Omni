// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class CustomTeam : ModuleRules
{
	public CustomTeam(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"AIModule",
				"DeveloperSettings",
			}
		);
	}
}