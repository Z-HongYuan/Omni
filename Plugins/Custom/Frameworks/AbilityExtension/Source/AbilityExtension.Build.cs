// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class AbilityExtension : ModuleRules
{
	public AbilityExtension(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"GameplayAbilities", "GameplayTags", "GameplayTasks",
				"MessageRouters",
				"EnhancedInput",
				"NetCore",
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"DeveloperSettings",
				"PhysicsCore",
				"Slate", "SlateCore"
			}
		);
	}
}