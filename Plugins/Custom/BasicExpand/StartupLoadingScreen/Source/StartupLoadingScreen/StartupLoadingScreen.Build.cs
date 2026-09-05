// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class StartupLoadingScreen : ModuleRules
{
    public StartupLoadingScreen(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "PreLoadScreen",
                "SlateCore",
                "Slate"
            }
        );
    }
}