#include "ImGuiDeveloperToolkit/ImGuiDeveloperToolkitSettings.h"

#include "Zakazane/Property.h"

FImGuiDeveloperToolkitFontSettings::FImGuiDeveloperToolkitFontSettings()
	: GlyphRanges{UImGuiDeveloperToolkitSettings::LatinGlyphRange.Name}
{
}

FImGuiDeveloperToolkitFontSettings::FImGuiDeveloperToolkitFontSettings(
	FUtf8String InName, const int32 InSize, TArray<FName> InGlyphRanges)
	: Name{MoveTemp(InName)}, Size{InSize}, GlyphRanges{MoveTemp(InGlyphRanges)}
{
}

FStringView FImGuiDeveloperToolkitFontSettings::GetName() const
{
	return Name;
}

int32 FImGuiDeveloperToolkitFontSettings::GetSize() const
{
	return Size;
}

const TArray<FName>& FImGuiDeveloperToolkitFontSettings::GetGlyphRanges() const
{
	return GlyphRanges;
}

void FImGuiDeveloperToolkitFontSettings::SetName(UImGuiDeveloperToolkitSettings& Settings, FString InName)
{
	// #TODO #Properties: can't give the path to the font name property directly, because Emit doesn't support
	// providing pointers to structure properties. This should be handled properly.
	static const FName PropertyName = GET_MEMBER_NAME_CHECKED(UImGuiDeveloperToolkitSettings, FontSettings);

	Zkz::EmitPropertyChangeNotifications(
		{{&Settings, PropertyName}}, (Name == InName), [this, &InName] { Name = MoveTemp(InName); });

	// #TODO_dontcommit: need to implement "Save to Defaults", see:
	// SSettingsSectionHeader::HandleSetAsDefaultButtonClicked

	// #TODO_dontcommit:
	// For properties saving to file I've now added this below, but see how this is handled when properties change
	// in the project settings window, I'd expect them to be saved automatically on property change.
	// Up: yes, this can be verified by breaking in UObject::SaveConfig and changing a property in project
	// settings. Handled property changed event:
	// 		PropertyChangedEvent = {const FPropertyChangedEvent &} {Property=0x0000025d3d47b860 Name="Size", MemberProperty=0x0000025d3d47dfc0 Name="FontSettings", ChangeType=16, ...}
	// 		Property = {FIntProperty *} 0x0000025d3d47b860 Name="Size"
	// 		MemberProperty = {FStructProperty *} 0x0000025d3d47dfc0 Name="FontSettings"
	// 		ChangeType = {unsigned int} 16
	// 		ObjectIteratorIndex = {int} -1
	// 		ArrayIndicesPerObject = {TArrayView<const TMap<FString, int, FDefaultSetAllocator, TDefaultMapHashableKeyFuncs<FString, int, 0>>, int>} Num=1
	// 		InstancesChanged = {TSet<UObject *, DefaultKeyFuncs<UObject *, 0>, FDefaultSetAllocator>} Empty
	// 		bFilterChangedInstances = {bool} false
	// 		TopLevelObjects = {TArrayView<const UObject *const, int>} Num=1

	Settings.SaveConfig();
}

void FImGuiDeveloperToolkitFontSettings::SetSize(UImGuiDeveloperToolkitSettings& Settings, int32 InSize)
{
	static const FName PropertyName = GET_MEMBER_NAME_CHECKED(UImGuiDeveloperToolkitSettings, FontSettings);

	Zkz::EmitPropertyChangeNotifications(
		{{&Settings, PropertyName}}, (Size == InSize), [this, &InSize] { Size = MoveTemp(InSize); });

	// #TODO_dontcommit: see comment in setname
	Settings.SaveConfig();
}

void FImGuiDeveloperToolkitFontSettings::SetGlyphRanges(
	UImGuiDeveloperToolkitSettings& Settings, TArray<FName> InGlyphRanges)
{
	static const FName PropertyName = GET_MEMBER_NAME_CHECKED(UImGuiDeveloperToolkitSettings, FontSettings);

	Zkz::EmitPropertyChangeNotifications(
		{{&Settings, PropertyName}},
		(GlyphRanges == InGlyphRanges),
		[this, &InGlyphRanges] { GlyphRanges = MoveTemp(InGlyphRanges); });

	// #TODO_dontcommit: see comment in setname
	Settings.SaveConfig();
}

const FImGuiDeveloperToolkitFontSettings
	UImGuiDeveloperToolkitSettings::FallbackFontSettings{"Roboto-Regular", 14, {LatinGlyphRange.Name}};
const FImGuiDeveloperToolkitFontGlyphRanges UImGuiDeveloperToolkitSettings::LatinGlyphRange{
	"Latin", "The quick brown fox jumps over the lazy dog", {{0x0020, 0x007F}}};

const TArray<FImGuiDeveloperToolkitFontGlyphRanges> UImGuiDeveloperToolkitSettings::BuiltinGlyphRanges{
	FImGuiDeveloperToolkitFontGlyphRanges{
		"Polish Diacritics",
		// ReSharper disable once StringLiteralTypo
		"Zażółć gęślą jaźń",
		{
			{0x0104, 0x0105},  // Ą, ą
			{0x0106, 0x0107},  // Ć, ć
			{0x0118, 0x0119},  // Ę, ę
			{0x0141, 0x0142},  // Ł, ł
			{0x0143, 0x0144},  // Ń, ń
			{0x00D3, 0x00D3},  // Ó
			{0x00F3, 0x00F3},  // ó
			{0x015A, 0x015B},  // Ś, ś
			{0x0179, 0x017A},  // Ź, ź
			{0x017B, 0x017C}   // Ż, ż
		}}};

UImGuiDeveloperToolkitSettings& UImGuiDeveloperToolkitSettings::Get()
{
	UImGuiDeveloperToolkitSettings* const Default = GetMutableDefault<UImGuiDeveloperToolkitSettings>();
	if (!ensure(IsValid(Default)))
	{
		static UImGuiDeveloperToolkitSettings Fallback;
		return Fallback;
	}

	return *Default;
}

void UImGuiDeveloperToolkitSettings::ResetFontSettings()
{
	FontSettings = FallbackFontSettings;

	// #TODO_dontcommit: see comment in setname
	SaveConfig();
}

TArray<FName> UImGuiDeveloperToolkitSettings::GetGlyphRangeNames() const
{
	TArray<FName> Result;

	Result.Reserve(1 + BuiltinGlyphRanges.Num() + UserGlyphRanges.Num());

	Result.Emplace(LatinGlyphRange.Name);
	Algo::Transform(BuiltinGlyphRanges, Result, &FImGuiDeveloperToolkitFontGlyphRanges::Name);
	Algo::Transform(UserGlyphRanges, Result, &FImGuiDeveloperToolkitFontGlyphRanges::Name);

	return Result;
}

const FImGuiDeveloperToolkitFontGlyphRanges* UImGuiDeveloperToolkitSettings::FindGlyphRangesByName(FName InName) const
{
	if (LatinGlyphRange.Name == InName)
	{
		return &LatinGlyphRange;
	}

	if (auto* const BuiltinFontGlyphRange =
			Algo::FindBy(BuiltinGlyphRanges, InName, &FImGuiDeveloperToolkitFontGlyphRanges::Name))
	{
		return BuiltinFontGlyphRange;
	}

	return Algo::FindBy(UserGlyphRanges, InName, &FImGuiDeveloperToolkitFontGlyphRanges::Name);
}
