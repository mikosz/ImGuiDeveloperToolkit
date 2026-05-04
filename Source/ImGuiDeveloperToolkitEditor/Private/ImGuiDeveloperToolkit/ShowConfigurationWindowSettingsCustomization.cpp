#include "ShowConfigurationWindowSettingsCustomization.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "ImGuiDeveloperToolkit/ImGuiDeveloperToolkitSubsystem.h"
#include "Zakazane/ReturnIfMacros.h"

#define LOCTEXT_NAMESPACE "ImguiDeveloperToolkit"

namespace ImGuiDeveloperToolkit::Editor
{

TSharedRef<IDetailCustomization> FShowConfigurationWindowSettingsCustomization::MakeInstance()
{
	return MakeShared<FShowConfigurationWindowSettingsCustomization>();
}

void FShowConfigurationWindowSettingsCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	static const auto ShowConfigurationWindowText =
		LOCTEXT("Settings.ShowConfigurationWindow", "Show Configuration Window");

	auto& CategoryBuilder = DetailBuilder.EditCategory("ImGuiDeveloperToolkitSettings");
	auto& Row = CategoryBuilder.AddCustomRow(ShowConfigurationWindowText);

	auto DetailFont = DetailBuilder.GetDetailFont();

	// clang-format off
	Row.WholeRowWidget[
		SNew(SButton)
			.OnClicked_Lambda(
				[]
				{
					auto* const DeveloperToolkit = UImGuiDeveloperToolkitSubsystem::Get();
					ZKZ_RETURN_IF_INVALID(DeveloperToolkit, FReply::Handled());

					DeveloperToolkit->ShowConfigurationWindow();
					
					return FReply::Handled();
				}
			)
		[
			SNew(STextBlock)
				.Text(ShowConfigurationWindowText)
				.Font(MoveTemp(DetailFont))
		]
	];
	// clang-format on
}

}  // namespace ImGuiDeveloperToolkit::Editor

#undef LOCTEXT_NAMESPACE
