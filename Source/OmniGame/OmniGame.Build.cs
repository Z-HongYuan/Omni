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
			"GameplayAbilities", //Character 公开 ASC 接口
			"ModularGameplay", //Pawn 初始化状态接口
			"CoreExtension", //项目游戏实例、本地玩家与控制器继承插件基础类
			"ExperienceSystem", //项目游戏模式继承体验驱动流程
			"ModularGameplayActors", //项目游戏模式与玩家控制器继承链所需的模块化 Actor 实现
			"CommonUI", //项目视口继承 CommonUI 输入路由
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"InputCore", "EnhancedInput", //
			"AbilityExtension", //访问插件 ASC 实现
		});
	}
}
