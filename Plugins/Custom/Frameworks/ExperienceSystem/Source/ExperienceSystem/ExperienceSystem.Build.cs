// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ExperienceSystem : ModuleRules
{
	public ExperienceSystem(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"GameFeatures", "ModularGameplay", "ModularGameplayActors", //模块化核心
				"GameplayAbilities", "GameplayTags", "CustomAbilitySystem", //自定义能力系统核心
				"AIModule", //AI模块
				"LoadingScreen", //加载屏模块
				"GameplayCameras", //游戏摄像机
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"EnhancedInput", "CustomInputSystem", //自定义输入系统
				"DeveloperSettings", //开发者设置模块
				"NetCore", //网络核心模块
			}
		);
	}
}