using UnrealBuildTool;

public class ImGuiDeveloperToolkitWidgets : ModuleRules
{
	public ImGuiDeveloperToolkitWidgets(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new[]
			{
				"Core",
				"ZakazaneUtilities"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new[]
			{
				"CoreUObject",
				"DeveloperSettings",
				"Engine",
				"Slate",
				"SlateCore",
				"ImGui",
				"ZakazaneUtilitiesEditor"
			}
		);
	}
}