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
			"CoreExtension", //项目游戏实例与本地玩家继承插件基础类
			"CommonUI", //项目视口继承 CommonUI 输入路由
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"InputCore", "EnhancedInput", //
		});
	}
}
