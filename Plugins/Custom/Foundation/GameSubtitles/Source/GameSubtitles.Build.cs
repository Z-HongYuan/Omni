// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class GameSubtitles : ModuleRules
{
	public GameSubtitles(ReadOnlyTargetRules Target) : base(Target)
	{
		// 公共头文件直接使用对象、引擎和 Slate 类型，向使用方公开这些依赖。
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"Overlay",
				"UMG",
				"MediaAssets",
				"MediaUtils",
				"GameplayTags"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Projects" // 通过插件管理器获取实际安装目录。
			}
		);
	}
}
