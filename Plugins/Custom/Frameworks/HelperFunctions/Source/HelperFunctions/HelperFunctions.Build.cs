// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class HelperFunctions : ModuleRules
{
	public HelperFunctions(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"GameplayTags", "GameAbilitySystem",
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"DeveloperSettings"
			}
		);
	}
}