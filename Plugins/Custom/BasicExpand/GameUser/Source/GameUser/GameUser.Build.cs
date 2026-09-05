// Copyright © 2026 张鸿源. All Rights Reserved.

using UnrealBuildTool;

public class GameUser : ModuleRules
{
	public GameUser(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

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