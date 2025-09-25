#include "ImGuiDeveloperToolkit/ImGuiDeveloperToolkitSettings.h"

#include "ImGuiDeveloperToolkit/ImGuiDeveloperToolkitSubsystem.h"

const FImGuiDeveloperToolkitFontConfiguration UImGuiDeveloperToolkitDefaultSettings::FallbackFontSettings{
	"Roboto-Regular", 14, static_cast<int32>(EImGuiDeveloperToolkitGlyphRanges::BasicLatin)};

UImGuiDeveloperToolkitDefaultSettings& UImGuiDeveloperToolkitDefaultSettings::Get()
{
	UImGuiDeveloperToolkitDefaultSettings* const Default = GetMutableDefault<UImGuiDeveloperToolkitDefaultSettings>();
	if (!ensure(IsValid(Default)))
	{
		static UImGuiDeveloperToolkitDefaultSettings Fallback;
		return Fallback;
	}

	return *Default;
}

void UImGuiDeveloperToolkitDefaultSettings::ResetFontSettings()
{
	FontSettings = FallbackFontSettings;
}

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
	ReconciledFontSettings.Name =
		(FontSettings.Name.IsEmpty() ? DefaultSettings->FontSettings.Name : FontSettings.Name);
	ReconciledFontSettings.Size = (FontSettings.Size <= 0 ? DefaultSettings->FontSettings.Size : FontSettings.Size);
	ReconciledFontSettings.GlyphRanges =
		(FontSettings.GlyphRanges == 0 ? DefaultSettings->FontSettings.GlyphRanges : FontSettings.GlyphRanges);

	return ReconciledFontSettings;
}

void UImGuiDeveloperToolkitUserSettings::SetFontSettings(FImGuiDeveloperToolkitFontConfiguration InFontSettings)
{
	FontSettings = MoveTemp(InFontSettings);
}
