// Copyright Epic Games, Inc. All Rights Reserved.

#include "ImGuiDeveloperToolkitSubsystemModule.h"

#include "ISettingsModule.h"
#include "ISettingsSection.h"
#include "ImGuiDeveloperToolkit/ImGuiDeveloperToolkitSettings.h"

#define LOCTEXT_NAMESPACE "ImGuiDeveloperToolkit"

namespace ImGuiDeveloperToolkit
{

void FImGuiDeveloperToolkitSubsystemModule::StartupModule()
{
	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");

	if (SettingsModule != nullptr)
	{
		// #TODO_dontcommit: document in unreal-notes
		ISettingsSectionPtr SettingsSection = SettingsModule->RegisterSettings(
			"Project",
			"Plugins",
			"ImGuiDeveloperToolkit",
			LOCTEXT("ImGuiDeveloperSettingsDisplayName", "ImGui Developer Toolkit"),
			LOCTEXT(
				"ImGuiDeveloperSettingsDescription",
				"Settings for the ImGuiDeveloperToolkit should be handled via the configuration window accessible from "
				"the toolkit's window menu bar.\n"
				"Use this window to handle set as default / reset to default options."),
			GetMutableDefault<UImGuiDeveloperToolkitSettings>());
	}
}

void FImGuiDeveloperToolkitSubsystemModule::ShutdownModule()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings("Project", "Plugins", "ImGui Developer Toolkit");
	}
}

IMPLEMENT_MODULE(FImGuiDeveloperToolkitSubsystemModule, ImGuiDeveloperToolkitSubsystem)

}  // namespace ImGuiDeveloperToolkit

#undef LOCTEXT_NAMESPACE
