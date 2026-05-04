// Copyright ZAKAZANE Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "ImGuiDeveloperToolkitSettings.h"
#include "Zakazane/Future.h"

#include "ImGuiDeveloperToolkitConfigurationWindow.generated.h"

struct ImFont;

USTRUCT()
struct IMGUIDEVELOPERTOOLKITSUBSYSTEM_API FImGuiDeveloperToolkitConfigurationWindow
{
	GENERATED_BODY()

	UPROPERTY(Transient, VisibleAnywhere)
	bool bShown = false;

	void Initialize();

	void Tick(float DeltaTime);

	ImFont* GetFont() const;

	void SetToolShown(FAnsiString ToolName, bool bToolShown);

	bool IsShown(const FAnsiString& ToolName) const;

private:
	UPROPERTY(Transient, VisibleAnywhere)
	TMap<FUtf8String, FUtf8String> AvailableFontPathsByName;

	UPROPERTY(VisibleAnywhere)
	bool bFontControlsEnabled = true;

	// #TODO_dontcommit: unused?
	UPROPERTY(VisibleAnywhere)
	FImGuiDeveloperToolkitFontSettings PreviousFontSettings;

	ImFont* DefaultFont = nullptr;

	ImFont* SelectedFont = nullptr;

	FDelegateHandle SetSelectedFontDelegateHandle = {};

#if WITH_ENGINE
	TStrongObjectPtr<UTexture2D> FontAtlasTexturePtr = nullptr;
#else
	TSharedPtr<FSlateBrush> FontAtlasTexturePtr = nullptr;
#endif

	float ShowResetFontPopup_S = -1.f;

	void TickFontSelector(float DeltaTime);

	void TickResetFontPopup(float DeltaTime);

	Zkz::TCancelableFuture<bool> LoadFonts();
};
