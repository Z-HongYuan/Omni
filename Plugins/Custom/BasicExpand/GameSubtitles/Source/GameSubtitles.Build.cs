// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class GameSubtitles : ModuleRules
{
	public GameSubtitles(ReadOnlyTargetRules Target) : base(Target)
	{
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"Overlay",
                "UMG",
				"MediaAssets",
				"MediaUtils",
				"GameplayTags"
			}
		);

        PublicIncludePathModuleNames.AddRange(
            new string[] {
                "UMG",
            }
        );
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
			}
		);
	}
}
