// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ImGuiDeveloperToolkitSubsystem : ModuleRules
{
	public ImGuiDeveloperToolkitSubsystem(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[]
			{
			}
		);


		PrivateIncludePaths.AddRange(
			new string[]
			{
			}
		);


		PublicDependencyModuleNames.AddRange(
			new[]
			{
				"Core",
				"ImGuiLibrary",
				"ZakazaneUtilities"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new[]
			{
				"CoreUObject",
				"DeveloperSettings",
				"Engine",
				"ImGui",
				"ImGuiDeveloperToolkitWidgets",
				"Slate",
				"SlateCore"
			}
		);

		if (Target.Type == TargetRules.TargetType.Editor) PrivateDependencyModuleNames.AddRange(["UnrealEd"]);

		DynamicallyLoadedModuleNames.AddRange(
			[
			]
		);
	}
}