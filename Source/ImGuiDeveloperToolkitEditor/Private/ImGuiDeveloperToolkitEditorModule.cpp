#include "ImGuiDeveloperToolkitEditorModule.h"

#include "ImGuiDeveloperToolkit/EditorCommands.h"
#include "ImGuiDeveloperToolkit/ImGuiDeveloperToolkitSettings.h"
#include "ImGuiDeveloperToolkit/ShowConfigurationWindowSettingsCustomization.h"
#include "Zakazane/Class.h"

void FImGuiDeveloperToolkitEditorModule::StartupModule()
{
	using namespace ImGuiDeveloperToolkit;

	RegisterEditorCommands();

	auto& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyEditorModule.RegisterCustomClassLayout(
		Zkz::GetClassName<UImGuiDeveloperToolkitSettings>().Get({}),
		FOnGetDetailCustomizationInstance::CreateStatic(
			&Editor::FShowConfigurationWindowSettingsCustomization::MakeInstance));
}

void FImGuiDeveloperToolkitEditorModule::ShutdownModule()
{
	using namespace ImGuiDeveloperToolkit;

	UnregisterEditorCommands();

	auto& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyEditorModule.UnregisterCustomClassLayout(Zkz::GetClassName<UImGuiDeveloperToolkitSettings>().Get({}));
}

IMPLEMENT_MODULE(FImGuiDeveloperToolkitEditorModule, ImGuiDeveloperToolkitEditor)
