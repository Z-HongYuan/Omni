// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class OmniEditor : ModuleRules
{
	public OmniEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Engine",
			"OmniGame",
			"Projects",
			"Slate",
			"SlateCore",
			"ToolMenus",
			"UnrealEd"
		});
	}
}