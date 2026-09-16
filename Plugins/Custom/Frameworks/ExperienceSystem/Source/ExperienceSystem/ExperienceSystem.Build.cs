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
				"GameplayAbilities", "GameplayTags", "AbilityExtension", //自定义能力系统核心
				"AIModule", //AI模块
				"LoadingScreen", //加载屏模块
				"GameplayCameras", //游戏摄像机
				"GameUser", //玩法定义公开会话请求类型
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"UMG", //玩法定义中的加载屏控件类型
				"EnhancedInput", //自定义输入系统
				"DeveloperSettings", //开发者设置模块
				"NetCore", //网络核心模块
			}
		);
	}
}
