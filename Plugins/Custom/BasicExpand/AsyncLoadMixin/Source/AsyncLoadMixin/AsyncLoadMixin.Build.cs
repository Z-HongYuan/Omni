// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class AsyncLoadMixin : ModuleRules
{
	public AsyncLoadMixin(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine"
			}
		);
	}
}