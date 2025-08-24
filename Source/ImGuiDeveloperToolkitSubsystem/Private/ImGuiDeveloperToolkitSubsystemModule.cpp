// Copyright Epic Games, Inc. All Rights Reserved.

#include "ImGuiDeveloperToolkitSubsystemModule.h"

#include "ISettingsModule.h"
#include "ImGuiDeveloperToolkit/ImGuiDeveloperToolkitConfiguration.h"

#define LOCTEXT_NAMESPACE "FImGuiDeveloperToolkitSubsystemModule"

namespace ImGuiDeveloperToolkit
{

void FImGuiDeveloperToolkitSubsystemModule::StartupModule()
{
	// #TODO_dontcommit
	// if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	// {
	// 	SettingsModule->RegisterSettings(
	// 		"Project",
	// 		"Plugins",
	// 		"ImGui Developer Toolkit",
	// 		NSLOCTEXT("ImGuiToolkit", "SettingsName", "ImGui Developer Toolkit"),
	// 		NSLOCTEXT("ImGuiToolkit", "SettingsDesc", "Settings for the ImGui Developer Toolkit plugin."),
	// 		GetMutableDefault<UImGuiDeveloperToolkitSettings>());
	// }
}

void FImGuiDeveloperToolkitSubsystemModule::ShutdownModule()
{
	// if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	// {
	// 	SettingsModule->UnregisterSettings("Project", "Plugins", "ImGui Developer Toolkit");
	// }
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FImGuiDeveloperToolkitSubsystemModule, ImGuiDeveloperToolkitSubsystem)

}  // namespace ImGuiDeveloperToolkit
