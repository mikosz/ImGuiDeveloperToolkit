// Copyright ZAKAZANE Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "ImGuiDeveloperToolkit/ImGuiDeveloperToolkitConfiguration.h"

#include "ImGuiDeveloperToolkitSettings.generated.h"

// ReSharper disable once CppUE4CodingStandardNamingViolationWarning
struct ImFont;

UCLASS(Config = "ImGuiDevelopersToolkit", DefaultConfig)
class UImGuiDeveloperToolkitDefaultSettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	/// Default font configuration for ImGui Developer Toolkit. Modify in the toolkit by selecting the default
	/// configuration.
	UPROPERTY(VisibleAnywhere, Config)
	FImGuiDeveloperToolkitFontConfiguration
		Font{"Roboto-Regular", 14, static_cast<int32>(EImGuiDeveloperToolkitGlyphRanges::BasicLatin)};
};

UCLASS(Config = "ImGuiDevelopersToolkit", PerObjectConfig)
class UImGuiDeveloperToolkitUserSettings : public UObject
{
	GENERATED_BODY()
public:
	static UImGuiDeveloperToolkitUserSettings& Get();

	UPROPERTY(Config)
	TArray<FString> OpenTools;

	const FImGuiDeveloperToolkitFontConfiguration& GetFontSettings() const;

	void SetFontSettings(const FImGuiDeveloperToolkitFontConfiguration& InFontSettings);

private:
	static TStrongObjectPtr<UImGuiDeveloperToolkitUserSettings> UserSettings;

	UPROPERTY(EditAnywhere, Config, meta = (AllowPrivateAccess))
	FImGuiDeveloperToolkitFontConfiguration FontSettings;

	UImGuiDeveloperToolkitUserSettings() = default;
};
