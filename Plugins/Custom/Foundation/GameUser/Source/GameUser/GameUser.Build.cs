// Copyright © 2026 张鸿源. All Rights Reserved.

using UnrealBuildTool;

public class GameUser : ModuleRules
{
	public GameUser(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		// 公共头文件使用对象、引擎与输入类型，并公开 OSS v1 接口。
		PublicDependencyModuleNames.Add("OnlineSubsystem");

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreOnline",
				"GameplayTags",
				"OnlineSubsystemUtils",
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreOnline",
				"CoreUObject",
				"Engine",
				"ApplicationCore",
				"InputCore",
			}
		);
	}
}