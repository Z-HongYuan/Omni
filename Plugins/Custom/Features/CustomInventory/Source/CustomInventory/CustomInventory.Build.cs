// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class CustomInventory : ModuleRules
{
	public CustomInventory(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"GameplayTags", "GameplayTasks", "CustomAbilitySystem",
				"NetCore",
				"IrisCore",
				"MessageRouters"
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