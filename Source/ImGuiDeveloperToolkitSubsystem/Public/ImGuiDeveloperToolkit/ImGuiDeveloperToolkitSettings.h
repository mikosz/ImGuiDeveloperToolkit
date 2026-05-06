// Copyright ZAKAZANE Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "ImGuiDeveloperToolkitSettings.generated.h"

// ReSharper disable once CppUE4CodingStandardNamingViolationWarning
struct ImFont;
class UImGuiDeveloperToolkitSettings;

USTRUCT()
struct IMGUIDEVELOPERTOOLKITSUBSYSTEM_API FImGuiDeveloperToolkitFontSettings
{
public:
	GENERATED_BODY();

	FImGuiDeveloperToolkitFontSettings();
	FImGuiDeveloperToolkitFontSettings(FUtf8String InName, const int32 InSize, TArray<FName> InGlyphRanges);

	FStringView GetName() const;

	int32 GetSize() const;

	const TArray<FName>& GetGlyphRanges() const;

	void SetName(UImGuiDeveloperToolkitSettings& Settings, FString InName);

	void SetSize(UImGuiDeveloperToolkitSettings& Settings, int32 InSize);

	void SetGlyphRanges(UImGuiDeveloperToolkitSettings& Settings, TArray<FName> InGlyphRanges);

private:
	UPROPERTY(EditAnywhere)
	FString Name;

	UPROPERTY(EditAnywhere, meta = (UIMin = 6, UIMax = 32))
	int32 Size = -1;

	UPROPERTY(EditAnywhere, meta = (GetOptions = "GetGlyphRangeNames"))
	TArray<FName> GlyphRanges;
};

// #TODO_dontcommit: test in cooked game!
// #TODO_dontcommit: is the Config = entry necessary?

USTRUCT()
struct FImGuiDeveloperToolkitFontGlyphRange
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	uint64 FirstGlyph;

	UPROPERTY(EditAnywhere)
	uint64 LastGlyph;
};

USTRUCT()
struct FImGuiDeveloperToolkitFontGlyphRanges
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FName Name;

	/// Test string for a given glyph range, to be displayed when changing a font, to see if it supports all
	/// necessary glyphs.
	UPROPERTY(EditAnywhere)
	FUtf8String TestString;

	UPROPERTY(EditAnywhere)
	TArray<FImGuiDeveloperToolkitFontGlyphRange> Ranges;
};

UCLASS(MinimalAPI, Config = "ImGuiDeveloperToolkit")
class UImGuiDeveloperToolkitSettings : public UObject
{
	GENERATED_BODY()
public:
	static const FImGuiDeveloperToolkitFontSettings FallbackFontSettings;

	static const FImGuiDeveloperToolkitFontGlyphRanges LatinGlyphRange;
	static const TArray<FImGuiDeveloperToolkitFontGlyphRanges> BuiltinGlyphRanges;

	UPROPERTY(EditAnywhere, Config, meta = (TitleProperty = "Name", ShowOnlyInnerProperties))
	TArray<FImGuiDeveloperToolkitFontGlyphRanges> UserGlyphRanges;

	UPROPERTY(EditAnywhere, Config)
	FImGuiDeveloperToolkitFontSettings FontSettings = FallbackFontSettings;

	UPROPERTY(EditAnywhere, Config)
	TMap<FAnsiString, bool> ToolsShownByName;

	static UImGuiDeveloperToolkitSettings& Get();

	void ResetFontSettings();

	UFUNCTION()
	TArray<FName> GetGlyphRangeNames() const;

	const FImGuiDeveloperToolkitFontGlyphRanges* FindGlyphRangesByName(FName InName) const;
};
