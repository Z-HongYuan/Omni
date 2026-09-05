// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ModularGameplayActors : ModuleRules
{
	public ModularGameplayActors(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"ModularGameplay",
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"AIModule",
			}
		);
	}
}