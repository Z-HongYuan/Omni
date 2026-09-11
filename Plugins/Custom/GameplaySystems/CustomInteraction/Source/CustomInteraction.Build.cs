// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class CustomInteraction : ModuleRules
{
	public CustomInteraction(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"GameplayAbilities", "GameplayTags", "GameplayTasks", "AbilityExtension",
				"UMG",
				"CustomIndicator"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
			}
		);
	}
}