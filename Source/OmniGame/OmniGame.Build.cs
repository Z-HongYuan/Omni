// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Omni : ModuleRules
{
	public Omni(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", // 基础模块引用
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
		});
	}
}