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
			"CoreUObject",
			"Engine", "UnrealEd" //公开的编辑器引擎类依赖
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"OmniGame",
			"Projects",
			"Slate",
			"SlateCore",
			"ToolMenus"
		});
	}
}
