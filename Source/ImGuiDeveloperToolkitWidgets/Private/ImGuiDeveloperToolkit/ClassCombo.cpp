#include "ImGuiDeveloperToolkit/ClassCombo.h"

#include "ImGuiDeveloperToolkit/Private/ImGuiDeveloperToolkitUtilities.h"
#include "Zakazane/ContinueIfMacros.h"
#include "imgui.h"

namespace ImGuiDeveloperToolkit::Widgets
{

bool ClassCombo(const char* Label, UClass& ParentClass, UClass*& SelectedClass, const FClassComboMask ClassFlags)
{
	if (!ImGui::BeginCombo(Label, IsValid(SelectedClass) ? IGDT_TEXT_TO_CSTR(SelectedClass->GetDisplayNameText()) : ""))
	{
		return false;
	}

	ON_SCOPE_EXIT
	{
		ImGui::EndCombo();
	};

	bool bSelectionChanged = false;

	TArray<UClass*> Classes;
	GetDerivedClasses(&ParentClass, Classes, true);
	Classes.Emplace(&ParentClass);
	Algo::SortBy(
		Classes,
		[](const UClass* const Class)
		{
			ZKZ_RETURN_IF_INVALID(Class, FString{});
			return Class->GetDisplayNameText().ToString();
		});

	for (UClass* const Class : Classes)
	{
		ZKZ_CONTINUE_IF_INVALID(Class);
		ZKZ_CONTINUE_IF(Class->HasAnyClassFlags(CLASS_NewerVersionExists));
		ZKZ_CONTINUE_IF(Class->HasAnyClassFlags(CLASS_Abstract) && !(ClassFlags & EClassComboFlags::IncludeAbstract));
		ZKZ_CONTINUE_IF(Class->HasAnyClassFlags(CLASS_Deprecated) && !(ClassFlags & EClassComboFlags::IncludeDeprecated));
		ZKZ_CONTINUE_IF(Class->IsNative() && (ClassFlags & EClassComboFlags::DiscardNative));
		ZKZ_CONTINUE_IF(!Class->IsNative() && (ClassFlags & EClassComboFlags::DiscardBlueprint));

		ImGui::PushID(Class);
		if (ImGui::Selectable(IGDT_TEXT_TO_CSTR(Class->GetDisplayNameText())))
		{
			SelectedClass = Class;
			bSelectionChanged = true;
		}
		ImGui::SetItemTooltip("%s", IGDT_TEXT_TO_CSTR(Class->GetToolTipText()));
		ImGui::PopID();
	}

	return bSelectionChanged;
}

bool ClassCombo(const char* Label, UClass& ParentClass, UClass* const& SelectedClass, const FClassComboMask ClassFlags)
{
	UClass* SelectedClassCopy = SelectedClass;

	ImGui::BeginDisabled();
	const bool bResult = ClassCombo(Label, ParentClass, SelectedClassCopy, ClassFlags);
	ImGui::EndDisabled();

	return bResult;
}

}  // namespace ImGuiDeveloperToolkit::Widgets
