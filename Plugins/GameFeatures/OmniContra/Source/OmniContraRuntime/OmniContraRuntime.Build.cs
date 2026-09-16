using UnrealBuildTool;

public class OmniContraRuntime : ModuleRules
{
	public OmniContraRuntime(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PublicDependencyModuleNames.AddRange(new[]
		{
			"Core", "CoreUObject", "Engine", "OmniGame", "AbilityExtension",
			"GameplayAbilities", "ModularGameplay", "UMG"
		});
		PrivateDependencyModuleNames.AddRange(new[]
		{
			"ExperienceSystem", "EnhancedInput", "InputCore", "GameplayTags", "GameplayTasks", "SlateCore"
		});
	}
}
