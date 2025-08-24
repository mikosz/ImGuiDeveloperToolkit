#include "ImGuiDeveloperToolkit/ImGuiDeveloperToolkitSettings.h"

#include "ImGuiDeveloperToolkit/ImGuiDeveloperToolkitSubsystem.h"

UImGuiDeveloperToolkitUserSettings& UImGuiDeveloperToolkitUserSettings::Get()
{
	if (!IsValid(UserSettings.Get()))
	{
		UserSettings.Reset(
			NewObject<UImGuiDeveloperToolkitUserSettings>(nullptr, "ImGuiDeveloperToolkitSettings_User"));
	}

	return *UserSettings;
}

const FImGuiDeveloperToolkitFontConfiguration& UImGuiDeveloperToolkitUserSettings::GetFontSettings() const
{
	const UImGuiDeveloperToolkitDefaultSettings* const DefaultSettings =
		GetDefault<UImGuiDeveloperToolkitDefaultSettings>();
	if (!IsValid(DefaultSettings))
	{
		return {};
	}

	FImGuiDeveloperToolkitFontConfiguration ReconciledFontSettings;
	ReconciledFontSettings.Name = (FontSettings.Name.IsEmpty() ? DefaultSettings->Font.Name : FontSettings.Name);
	ReconciledFontSettings.Size = (FontSettings.Size <= 0 ? DefaultSettings->Font.Size : FontSettings.Size);
	ReconciledFontSettings.GlyphRanges =
		(FontSettings.GlyphRanges == 0 ? DefaultSettings->Font.GlyphRanges : FontSettings.GlyphRanges);

	return ReconciledFontSettings;
}

void UImGuiDeveloperToolkitUserSettings::SetFontSettings(const FImGuiDeveloperToolkitFontConfiguration& InFontSettings)
{
	FontSettings = InFontSettings;
}
