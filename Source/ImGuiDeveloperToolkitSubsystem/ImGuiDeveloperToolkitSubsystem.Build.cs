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
				// ... add public include paths required here ...
			}
		);


		PrivateIncludePaths.AddRange(
			new string[]
			{
				// ... add other private include paths required here ...
			}
		);


		PublicDependencyModuleNames.AddRange(
			new[]
			{
				"Core",
				"ImGuiLibrary"
				// ... add other public dependencies that you statically link with here ...
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new[]
			{
				"CoreUObject",
				"Engine",
				"ImGui",
				"ImGuiDeveloperToolkitWidgets",
				"Slate",
				"SlateCore"
				// ... add private dependencies that you statically link with here ...	
			}
		);

		if (Target.Type == TargetRules.TargetType.Editor) PrivateDependencyModuleNames.AddRange(["UnrealEd"]);

		DynamicallyLoadedModuleNames.AddRange(
			[
				// ... add any modules that your module loads dynamically here ...
			]
		);
	}
}