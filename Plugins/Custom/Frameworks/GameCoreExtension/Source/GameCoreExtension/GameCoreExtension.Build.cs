// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class GameCoreExtension : ModuleRules
{
	public GameCoreExtension(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"GameFeatures", "ModularGameplay", "ModularGameplayActors", //模块化核心
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"CommonUI", "GameUI", //增强UI核心
				"GameUser", //用户管理模块
				"GameplayTags", //游戏标签模块
			}
		);
	}
}