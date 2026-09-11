// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class OmniGame : ModuleRules
{
	public OmniGame(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core", "CoreUObject", "Engine", //基础模块
			"GameplayTags", //
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"InputCore", "EnhancedInput", //
		});
	}
}