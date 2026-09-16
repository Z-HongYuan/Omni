// Copyright © 2026 张鸿源. All Rights Reserved.

using UnrealBuildTool;

public class ExperienceSystemEditor : ModuleRules
{
	public ExperienceSystemEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PrivateDependencyModuleNames.Add("Core");
	}
}
