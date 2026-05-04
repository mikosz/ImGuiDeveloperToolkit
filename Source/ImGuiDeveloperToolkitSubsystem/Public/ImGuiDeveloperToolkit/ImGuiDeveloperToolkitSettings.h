// Copyright ZAKAZANE Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "ImGuiDeveloperToolkitSettings.generated.h"

// ReSharper disable once CppUE4CodingStandardNamingViolationWarning
struct ImFont;
class UImGuiDeveloperToolkitSettings;

UENUM()
enum class EImGuiDeveloperToolkitGlyphRanges : uint8
{
	Default = 0 UMETA(Hidden),
	/// Basic Latin letters, digits, punctuation
	BasicLatin = 0b1,
	/// Polish national diacritics
	Polish = 0b1 << 1,
};

ENUM_CLASS_FLAGS(EImGuiDeveloperToolkitGlyphRanges);

USTRUCT()
struct IMGUIDEVELOPERTOOLKITSUBSYSTEM_API FImGuiDeveloperToolkitFontSettings
{
public:
	GENERATED_BODY();

	FImGuiDeveloperToolkitFontSettings() = default;
	FImGuiDeveloperToolkitFontSettings(FUtf8String InName, const int32 InSize, int32 InGlyphRanges);

	FStringView GetName() const;

	int32 GetSize() const;

	int32 GetGlyphRanges() const;

	void SetName(UImGuiDeveloperToolkitSettings& Settings, FString InName);

	void SetSize(UImGuiDeveloperToolkitSettings& Settings, int32 InSize);

	void SetGlyphRanges(UImGuiDeveloperToolkitSettings& Settings, int32 InGlyphRanges);

private:
	UPROPERTY(EditAnywhere)
	FString Name;

	UPROPERTY(EditAnywhere, meta = (UIMin = 6, UIMax = 32))
	int32 Size = -1;

	UPROPERTY(
		EditAnywhere,
		meta = (Bitmask, BitmaskEnum = "/Script/ImGuiDeveloperToolkitSubsystem.EImGuiDeveloperToolkitGlyphRanges"))
	int32 GlyphRanges = 0;
};

// #TODO_dontcommit: test in cooked game!
// #TODO_dontcommit: is the Config = entry necessary?

UCLASS(MinimalAPI, Config = "ImGuiDeveloperToolkit")
class UImGuiDeveloperToolkitSettings : public UObject
{
	GENERATED_BODY()
public:
	static const FImGuiDeveloperToolkitFontSettings FallbackFontSettings;

	UPROPERTY(EditAnywhere, Config)
	FImGuiDeveloperToolkitFontSettings FontSettings = FallbackFontSettings;

	UPROPERTY(EditAnywhere, Config)
	TMap<FAnsiString, bool> ToolsShownByName;

	static UImGuiDeveloperToolkitSettings& Get();

	void ResetFontSettings();
};
