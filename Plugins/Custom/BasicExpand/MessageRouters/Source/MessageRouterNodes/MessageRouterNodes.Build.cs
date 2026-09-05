// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MessageRouterNodes : ModuleRules
{
    public MessageRouterNodes(ReadOnlyTargetRules Target) : base(Target)
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
                "Engine",
                "BlueprintGraph",
                "UnrealEd",
                "BlueprintGraph",
                "KismetCompiler",
                "MessageRouters"
            }
        );
    }
}