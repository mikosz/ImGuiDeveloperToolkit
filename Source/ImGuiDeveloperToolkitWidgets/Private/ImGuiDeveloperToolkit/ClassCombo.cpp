#include "ImGuiDeveloperToolkit/ClassCombo.h"

#include "ImGuiDeveloperToolkit/Private/ImGuiDeveloperToolkitUtilities.h"
#include "Zakazane/FindAllAssetsByType.h"
#include "imgui.h"

namespace ImGuiDeveloperToolkit::Widgets
{

bool ClassCombo(const char* Label, const UClass& ParentClass, UClass*& SelectedClass)
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

	Zkz::ForEachAssetByType(
		&ParentClass,
		[&bSelectionChanged, &SelectedClass](const FAssetData& AssetData)
		{
			UClass* const AssetClass = AssetData.GetClass(EResolveClass::Yes);
			ZKZ_RETURN_IF_INVALID(AssetClass, true);

			if (ImGui::Selectable(IGDT_TEXT_TO_CSTR(AssetClass->GetDisplayNameText())))
			{
				SelectedClass = AssetClass;
				bSelectionChanged = true;
			}
			
			return true;
		});
	
	return bSelectionChanged;
}

bool ClassCombo(const char* Label, const UClass& ParentClass, UClass* const& SelectedClass)
{
	UClass* SelectedClassCopy = SelectedClass;

	ImGui::BeginDisabled();
	const bool bResult = ClassCombo(Label, ParentClass, SelectedClassCopy);
	ImGui::EndDisabled();

	return bResult;
}

}  // namespace ImGuiDeveloperToolkit::Widgets
