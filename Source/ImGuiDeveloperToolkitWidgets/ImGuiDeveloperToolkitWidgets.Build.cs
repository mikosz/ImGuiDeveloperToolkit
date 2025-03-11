using UnrealBuildTool;

public class ImGuiDeveloperToolkitWidgets : ModuleRules
{
	public ImGuiDeveloperToolkitWidgets(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new[]
			{
				"Core"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"ImGui"
			}
		);
	}
}