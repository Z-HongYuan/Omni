// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class OmniGame : ModuleRules
{
	public OmniGame(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"CommonUI",
			"UMG", "GameUI", "GameUIExtension", // UI系统
			"GameCoreExtension",
			"ModularGameplayActors", "ExperienceSystem", "AIModule",
			"GameplayAbilities", "GameplayTags", "GameplayTasks", "GameAbilitySystem", //角色需要转发 ASC 与实现 IAbilitySystemInterface
			"GameInputSystem", //输入组件
			"GameplayCameras", //相机系统
			"ModularGameplay", "GameFeatures"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"InputCore",
			"EnhancedInput"
		});
	}
}