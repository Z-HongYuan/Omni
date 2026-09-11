// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class PocketWorlds : ModuleRules
{
	public PocketWorlds(ReadOnlyTargetRules Target) : base(Target)
	{
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
			}
		);

		// 查询当前渲染接口支持的纹理尺寸上限。
		PrivateDependencyModuleNames.Add("RHI");
	}
}