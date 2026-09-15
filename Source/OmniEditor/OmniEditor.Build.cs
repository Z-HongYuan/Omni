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
			"EditorSubsystem",
			"Engine", "UnrealEd" //公开的编辑器引擎类依赖
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"OmniGame",
			"Projects",
			"AssetRegistry", "CollectionManager", //外部固定引用资产移动、重命名提醒
			"Slate",
			"SlateCore",
			"ToolMenus"
		});
	}
}