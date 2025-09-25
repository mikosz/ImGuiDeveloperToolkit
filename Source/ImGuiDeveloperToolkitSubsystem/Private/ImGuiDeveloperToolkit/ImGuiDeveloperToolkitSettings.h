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
	static const FImGuiDeveloperToolkitFontConfiguration FallbackFontSettings;

	/// Default font configuration for ImGui Developer Toolkit. Modify in the toolkit by selecting the default
	/// configuration.
	UPROPERTY(VisibleAnywhere, Config)
	FImGuiDeveloperToolkitFontConfiguration FontSettings = FallbackFontSettings;

	static UImGuiDeveloperToolkitDefaultSettings& Get();

	void ResetFontSettings();
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

	FImGuiDeveloperToolkitFontConfiguration& GetUserFontSettings();

	void SetFontSettings(FImGuiDeveloperToolkitFontConfiguration InFontSettings);

private:
	static TStrongObjectPtr<UImGuiDeveloperToolkitUserSettings> UserSettings;

	UPROPERTY(EditAnywhere, Config, meta = (AllowPrivateAccess))
	FImGuiDeveloperToolkitFontConfiguration FontSettings;

	UImGuiDeveloperToolkitUserSettings() = default;
};
