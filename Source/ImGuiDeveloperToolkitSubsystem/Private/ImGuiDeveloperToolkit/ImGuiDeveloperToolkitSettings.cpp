#include "ImGuiDeveloperToolkit/ImGuiDeveloperToolkitSettings.h"

#include "ImGuiDeveloperToolkit/ImGuiDeveloperToolkitSubsystem.h"
#include "Zakazane/Property.h"
#include "Zakazane/ReturnIfMacros.h"

FImGuiDeveloperToolkitFontSettings::FImGuiDeveloperToolkitFontSettings(
	FUtf8String InName, const int32 InSize, const int32 InGlyphRanges)
	: Name{MoveTemp(InName)}, Size{InSize}, GlyphRanges{InGlyphRanges}
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

int32 FImGuiDeveloperToolkitFontSettings::GetGlyphRanges() const
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

void FImGuiDeveloperToolkitFontSettings::SetGlyphRanges(UImGuiDeveloperToolkitSettings& Settings, int32 InGlyphRanges)
{
	static const FName PropertyName = GET_MEMBER_NAME_CHECKED(UImGuiDeveloperToolkitSettings, FontSettings);

	Zkz::EmitPropertyChangeNotifications(
		{{&Settings, PropertyName}},
		(GlyphRanges == InGlyphRanges),
		[this, &InGlyphRanges] { GlyphRanges = MoveTemp(InGlyphRanges); });

	// #TODO_dontcommit: see comment in setname
	Settings.SaveConfig();
}

const FImGuiDeveloperToolkitFontSettings UImGuiDeveloperToolkitSettings::FallbackFontSettings{
	"Roboto-Regular", 14, static_cast<int32>(EImGuiDeveloperToolkitGlyphRanges::BasicLatin)};

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
