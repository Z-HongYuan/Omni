using UnrealBuildTool;

public class OmniContraEditor : ModuleRules
{
	public OmniContraEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PrivateDependencyModuleNames.AddRange(new[]
		{
			"Core", "CoreUObject", "Engine", "UnrealEd", "OmniGame", "OmniContraRuntime",
			"AbilityExtension", "ExperienceSystem", "GameplayAbilities", "GameplayTags",
			"EnhancedInput", "InputCore", "ModularGameplay"
		});
	}
}