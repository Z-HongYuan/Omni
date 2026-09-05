// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class AdvancedUIExtension : ModuleRules
{
	public AdvancedUIExtension(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"UMG",
				"GameplayTags",
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
			}
		);
	}
}