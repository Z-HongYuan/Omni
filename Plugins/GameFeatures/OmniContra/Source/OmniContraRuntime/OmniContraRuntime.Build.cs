using UnrealBuildTool;

public class OmniContraRuntime : ModuleRules
{
	public OmniContraRuntime(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PrivateDependencyModuleNames.Add("Core");
	}
}
