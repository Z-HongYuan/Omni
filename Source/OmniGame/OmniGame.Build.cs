// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class OmniGame : ModuleRules
{
	public OmniGame(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// Modules whose headers are exposed through Public/
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine"
		});

		// Modules only used by implementation files in Private/
		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"InputCore",
			"EnhancedInput"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
