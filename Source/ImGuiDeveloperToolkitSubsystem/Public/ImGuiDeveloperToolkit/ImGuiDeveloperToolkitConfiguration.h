// Copyright ZAKAZANE Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "ImGuiDeveloperToolkitConfiguration.generated.h"

// ReSharper disable once CppUE4CodingStandardNamingViolationWarning
struct ImFont;

UENUM()
enum class EImGuiDeveloperToolkitGlyphRanges : uint8
{
	None = 0 UMETA(Hidden),
	/// Basic Latin letters, digits, punctuation
	BasicLatin = 0b1,
	/// Polish national diacritics
	Polish = 0b1 << 1,
};

ENUM_CLASS_FLAGS(EImGuiDeveloperToolkitGlyphRanges);

USTRUCT()
struct IMGUIDEVELOPERTOOLKITSUBSYSTEM_API FImGuiDeveloperToolkitFont
{
	GENERATED_BODY();

	UPROPERTY(VisibleAnywhere)
	FUtf8String Name;

	UPROPERTY(VisibleAnywhere)
	int32 Size = -1;

	ImFont* Font = nullptr;
};

USTRUCT()
struct IMGUIDEVELOPERTOOLKITSUBSYSTEM_API FImGuiDeveloperToolkitConfiguration
{
	GENERATED_BODY()

	UPROPERTY(Transient, VisibleAnywhere)
	bool bShown = false;

	void Initialize();

	void Tick(float DeltaTime);

	void SetFont(const FUtf8String& Name, int32 Size, EImGuiDeveloperToolkitGlyphRanges GlyphRanges);

	ImFont* GetFont() const;

	void SetShown(const FAnsiString& ToolName, bool bToolShown);

	bool IsShown(const FAnsiString& ToolName) const;

private:
	UPROPERTY(Transient, VisibleAnywhere)
	TMap<FUtf8String, FUtf8String> AvailableFontPathsByName;

	UPROPERTY(VisibleAnywhere)
	bool bFontControlsEnabled = true;

	UPROPERTY(VisibleAnywhere)
	FImGuiDeveloperToolkitFont SelectedFont;

	UPROPERTY(VisibleAnywhere, meta = (Bitmask, BitmaskEnum = "EImGuiDeveloperToolkitGlyphRanges"))
	int32 SelectedGlyphRanges = 0;

	UPROPERTY(VisibleAnywhere)
	FImGuiDeveloperToolkitFont DefaultFont;

	UPROPERTY(VisibleAnywhere)
	FImGuiDeveloperToolkitFont PreviousFont;

	FDelegateHandle SetSelectedFontDelegateHandle = {};

#if WITH_ENGINE
	TStrongObjectPtr<UTexture2D> FontAtlasTexturePtr = nullptr;
#else
	TSharedPtr<FSlateBrush> FontAtlasTexturePtr = nullptr;
#endif

	float ShowResetFontPopup_S = -1.f;

	void TickFontSelector(float DeltaTime);

	void TickResetFontPopup(float DeltaTime);

	TFuture<bool> LoadFonts();
};
