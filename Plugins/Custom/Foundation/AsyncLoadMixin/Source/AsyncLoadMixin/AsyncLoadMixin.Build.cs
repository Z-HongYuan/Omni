// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class AsyncLoadMixin : ModuleRules
{
	public AsyncLoadMixin(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		// 公开头文件使用这些模块的类型与模板，依赖需要传递给使用方。
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine"
			}
		);
	}
}