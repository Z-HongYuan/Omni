// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class GameAbilitySystem : ModuleRules
{
	public GameAbilitySystem(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"GameplayAbilities", "GameplayTags", "GameplayTasks",
				"MessageRouters",
				"NetCore",
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"DeveloperSettings",
				"PhysicsCore"
			}
		);
	}
}