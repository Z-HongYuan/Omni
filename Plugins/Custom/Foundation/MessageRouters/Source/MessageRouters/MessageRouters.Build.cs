// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MessageRouters : ModuleRules
{
	public MessageRouters(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"GameplayTags",
			}
		);
	}
}
