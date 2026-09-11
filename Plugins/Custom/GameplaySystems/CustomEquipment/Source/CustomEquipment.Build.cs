// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class CustomEquipment : ModuleRules
{
	public CustomEquipment(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"ModularGameplay",
				"GameplayAbilities", "GameplayTags", "AbilityExtension",
				"CustomInventory",
				"Niagara",
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
				"NetCore",
				"IrisCore"
			}
		);
	}
}