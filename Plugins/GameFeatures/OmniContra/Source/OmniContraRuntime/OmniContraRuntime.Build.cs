using UnrealBuildTool;

public class OmniContraRuntime : ModuleRules
{
	public OmniContraRuntime(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"ExperienceSystem"
		});
	}
}
