#include "ImGuiDeveloperToolkit/ImGuiDeveloperToolkitConfigurationWindow.h"

#include "ImGuiContext.h"
#include "ImGuiDeveloperToolkit/ImGuiDeveloperToolkitWindow.h"
#include "ImGuiDeveloperToolkit/Private/EnumValueRange.h"
#include "ImGuiDeveloperToolkit/Text.h"
#include "ImageUtils.h"
#include "String/Join.h"
#include "Zakazane/ContinueIfMacros.h"
#include "Zakazane/ReturnIfMacros.h"

// #TODO_dontcommit: a bunch of utf8 <-> Fstring casts in this file that we don't want

namespace ImGuiDeveloperToolkitConfigurationPrivate
{

TArray<ImWchar> MakeGlyphRanges(const TArray<FName>& GlyphRangeNames)
{
	using namespace ImGuiDeveloperToolkit::Private;

	TArray<ImWchar> Ranges;

	for (const FName GlyphRangeName : GlyphRangeNames)
	{
		const auto* const GlyphRanges = UImGuiDeveloperToolkitSettings::Get().FindGlyphRangesByName(GlyphRangeName);
		ZKZ_CONTINUE_IF(GlyphRanges == nullptr);

		for (const auto [FirstGlyph, LastGlyph] : GlyphRanges->Ranges)
		{
			Ranges.Emplace(FirstGlyph);
			Ranges.Emplace(LastGlyph);
		}
	}

	if (!Ranges.IsEmpty())
	{
		Ranges.Emplace(0);
	}

	return Ranges;
}

bool TickFontCombo(
	UImGuiDeveloperToolkitSettings& SettingsObject, const TMap<FUtf8String, FUtf8String>& AvailableFontPathsByName)
{
	auto& FontSettings = SettingsObject.FontSettings;
	bool bResult = false;

	if (!AvailableFontPathsByName.IsEmpty())
	{
		if (ImGui::BeginCombo(
				"Font",
				FontSettings.GetName().IsEmpty() ? "Select font"
												 : reinterpret_cast<const char*>(*FUtf8String{FontSettings.GetName()})))
		{
			for (const auto& [Name, Path] : AvailableFontPathsByName)
			{
				if (ImGui::Selectable(reinterpret_cast<const char*>(*Name), FontSettings.GetName() == FString{Name}))
				{
					bResult = true;
					FontSettings.SetName(SettingsObject, FString{Name});
				}
			}

			ImGui::EndCombo();
		}
	}

	return bResult;
}

bool TickGlyphRangesSelectables(
	UImGuiDeveloperToolkitSettings& SettingsObject,
	TArrayView<const FImGuiDeveloperToolkitFontGlyphRanges> GlyphRangesArray)
{
	auto& FontSettings = SettingsObject.FontSettings;

	for (const auto& GlyphRanges : GlyphRangesArray)
	{
		// #TODO #DeveloperToolkit: consider changing to a popup with multiple checkboxes, this would allow
		// to only show the reset button when changes committed.
		if (ImGui::Selectable(
				reinterpret_cast<const char*>(*GlyphRanges.Name.ToUtf8String()),
				FontSettings.GetGlyphRanges().Contains(GlyphRanges.Name)))
		{
			auto NewGlyphRanges = FontSettings.GetGlyphRanges();
			NewGlyphRanges.Emplace(GlyphRanges.Name);
			FontSettings.SetGlyphRanges(SettingsObject, MoveTemp(NewGlyphRanges));
			return true;
		}
	}

	return false;
}

bool TickGlyphRangesCombo(UImGuiDeveloperToolkitSettings& SettingsObject)
{
	const auto& FontSettings = SettingsObject.FontSettings;

	auto PreviewString = TUtf8StringBuilder<128>{
		InPlace, UE::String::JoinBy(FontSettings.GetGlyphRanges(), &FName::ToUtf8String, UTF8TEXT(", "))};

	const auto bComboOpen = ImGui::BeginCombo(
		"Glyph ranges", reinterpret_cast<const char*>(*PreviewString), ImGuiComboFlags_WidthFitPreview);
	ZKZ_RETURN_IF(!bComboOpen, false);
	const auto ComboGuard = Zkz::FScopedExecution{&ImGui::EndCombo};

	TickGlyphRangesSelectables(SettingsObject, {SettingsObject.LatinGlyphRange});
	TickGlyphRangesSelectables(SettingsObject, SettingsObject.BuiltinGlyphRanges);
	TickGlyphRangesSelectables(SettingsObject, SettingsObject.UserGlyphRanges);

	return false;
}

bool TickFontSizeCombo()
{
	auto& SettingsObject = UImGuiDeveloperToolkitSettings::Get();
	auto& FontSettings = SettingsObject.FontSettings;

	int32 FontSize = FontSettings.GetSize();
	const bool bSizeChanged = ImGui::SliderInt("Font size", &FontSize, 8, 32);

	if (bSizeChanged)
	{
		FontSettings.SetSize(SettingsObject, FontSize);
		return true;
	}

	return false;
}

}  // namespace ImGuiDeveloperToolkitConfigurationPrivate

void FImGuiDeveloperToolkitConfigurationWindow::Initialize()
{
	using namespace ImGuiDeveloperToolkitConfigurationPrivate;

	const FString EngineFontsDir = FPaths::EngineContentDir() / TEXT("Slate/Fonts");
	IFileManager& FileManager = IFileManager::Get();

	TArray<FString> AvailableFontsStr;
	FileManager.FindFilesRecursive(AvailableFontsStr, *EngineFontsDir, TEXT("*.ttf"), true, false, false);
	Algo::Transform(
		AvailableFontsStr,
		AvailableFontPathsByName,
		[](const FString& FontPathStr)
		{
			FUtf8String Name{StringCast<UTF8CHAR>(*FPaths::GetBaseFilename(FontPathStr))};
			FUtf8String Path{StringCast<UTF8CHAR>(*FontPathStr)};
			return MakeTuple(MoveTemp(Name), MoveTemp(Path));
		});

	// #TODO_dontcommit: not sure what to do with this
	// DefaultFontConfiguration.Name = FPaths::GetBaseFilename(FImGuiContext::GetDefaultFontPath());
	// DefaultFontConfiguration.Size = FImGuiContext::DefaultFontSize;
	//
	// if (SelectedFontConfiguration.Name.IsEmpty())
	// {
	// 	SelectedFontConfiguration = DefaultFontConfiguration;
	// }
}

void FImGuiDeveloperToolkitConfigurationWindow::Tick(const float DeltaTime)
{
	using namespace ImGuiDeveloperToolkit;

	ZKZ_RETURN_IF(!bShown);

	ON_SCOPE_EXIT
	{
		ImGui::End();
	};

	SetNextWindowPosAndSizeWithinMainViewport(ImVec2{.7f, .2f}, ImVec2{.25f, .25f}, ImGuiCond_FirstUseEver);

	if (ImGui::Begin("Configuration", &bShown))
	{
		ImGui::ShowStyleSelector("Theme");
		TickFontSelector(DeltaTime);
	}
}

ImFont* FImGuiDeveloperToolkitConfigurationWindow::GetFont() const
{
	return SelectedFont;
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void FImGuiDeveloperToolkitConfigurationWindow::SetToolShown(FAnsiString ToolName, const bool bToolShown)
{
	using namespace ImGuiDeveloperToolkitConfigurationPrivate;
	UImGuiDeveloperToolkitSettings::Get().ToolsShownByName.Emplace(MoveTemp(ToolName), bToolShown);
}

// ReSharper disable once CppMemberFunctionMayBeStatic
bool FImGuiDeveloperToolkitConfigurationWindow::IsShown(const FAnsiString& ToolName) const
{
	using namespace ImGuiDeveloperToolkitConfigurationPrivate;
	return UImGuiDeveloperToolkitSettings::Get().ToolsShownByName.FindOrAdd(ToolName, false);
}

void FImGuiDeveloperToolkitConfigurationWindow::TickFontSelector(const float DeltaTime)
{
	using namespace ImGuiDeveloperToolkitConfigurationPrivate;

	bool bFontChanged = false;
	bool bGlyphRangesChanged = false;
	bool bSizeChanged = false;

	{
		ImGui::BeginDisabled(!bFontControlsEnabled);
		const auto ScopedDisabled = Zkz::FScopedExecution(&ImGui::EndDisabled);

		UImGuiDeveloperToolkitSettings& SettingsObject = UImGuiDeveloperToolkitSettings::Get();

		// #TODO_dontcommit: resetting still doesn't work

		bFontChanged = TickFontCombo(SettingsObject, AvailableFontPathsByName);
		bGlyphRangesChanged = TickGlyphRangesCombo(SettingsObject);
		bSizeChanged = TickFontSizeCombo();
	}

	if (bFontControlsEnabled && (bFontChanged || bGlyphRangesChanged || bSizeChanged))
	{
		Zkz::IfNotCanceled(
			LoadFonts(),
			[this,
			 bFontChanged,
			 bGlyphRangesChanged,
			 ScopedFontControlsDisabled = Zkz::TScopedAssignment{bFontControlsEnabled, false}](const bool bSuccessful)
			{
				if (!bSuccessful || !(bFontChanged || bGlyphRangesChanged))
				{
					return;
				}

				constexpr float KeepFontAutoDeny_S = 10.0f;
				ShowResetFontPopup_S = KeepFontAutoDeny_S;
			});
	}

	TickKeepFontPopup(DeltaTime);
}

void FImGuiDeveloperToolkitConfigurationWindow::TickKeepFontPopup(const float DeltaTime)
{
	if (ShowResetFontPopup_S > 0.f)
	{
		ImGui::OpenPopup("Keep font?");

		const auto DefaultFontScopeGuard = ImGuiDeveloperToolkit::Widgets::PushFont(DefaultFont);

		if (ImGui::BeginPopupModal("Keep font?"))
		{
			ImGui::Text("Example text:");

			ImGui::NewLine();

			{
				const UImGuiDeveloperToolkitSettings& SettingsObject = UImGuiDeveloperToolkitSettings::Get();
				for (const auto GlyphRangesName : SettingsObject.FontSettings.GetGlyphRanges())
				{
					const auto* const GlyphRanges = SettingsObject.FindGlyphRangesByName(GlyphRangesName);
					ZKZ_CONTINUE_IF(GlyphRanges == nullptr);

					ImGui::Text("%s", reinterpret_cast<const char*>(*GlyphRangesName.ToUtf8String()));
					ImGui::SameLine();

					{
						const auto SelectedFontScopeGuard = ImGuiDeveloperToolkit::Widgets::PushFont(SelectedFont);
						ImGui::Text("%s", reinterpret_cast<const char*>(*GlyphRanges->TestString));
					}
				}
			}

			ImGui::NewLine();

			ImGui::Text("Keep selected font?");

			bool bClosePopup = false;

			if (ImGui::Button("Yes"))
			{
				bClosePopup = true;
			}

			ImGui::SameLine();

			FAnsiStringBuilderBase NoBuilder;
			NoBuilder.Appendf("No (%d)", FMath::FloorToInt32(ShowResetFontPopup_S));
			ShowResetFontPopup_S -= DeltaTime;

			if (ImGui::Button(NoBuilder.ToString()) || ShowResetFontPopup_S <= 0.f)
			{
				UImGuiDeveloperToolkitSettings::Get().ResetFontSettings();
				bClosePopup = true;
			}

			if (bClosePopup)
			{
				ShowResetFontPopup_S = -1.f;
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
	}
}

Zkz::TCancelableFuture<bool> FImGuiDeveloperToolkitConfigurationWindow::LoadFonts()
{
	using namespace ImGuiDeveloperToolkitConfigurationPrivate;

	const TSharedPtr<FImGuiContext> ImGuiContext = FImGuiContext::Get(ImGui::GetCurrentContext());
	if (!ImGuiContext.IsValid())
	{
		return Zkz::MakeImmediatePromise<bool>(false);
	}

	const auto Promise = MakeShared<Zkz::TScopedPromise<bool>>();

	SetSelectedFontDelegateHandle = ImGuiContext->OnPreFrame.AddLambda(
		[this, ImGuiContext, Promise]
		{
			const ImGuiIO& IO = ImGui::GetIO();

			if (IO.Fonts == nullptr)
			{
				Promise->SetValue(false);
				return;
			}

			const auto& FontSettings = UImGuiDeveloperToolkitSettings::Get().FontSettings;

			const FUtf8String* FontPath = AvailableFontPathsByName.Find(FUtf8String{FontSettings.GetName()});
			const FUtf8String DefaultFontPath{StringCast<UTF8CHAR>(*FImGuiContext::GetDefaultFontPath())};

			IO.Fonts->Clear();

			const auto GlyphRanges = MakeGlyphRanges(FontSettings.GetGlyphRanges());

			DefaultFont = IO.Fonts->AddFontFromFileTTF(
				reinterpret_cast<const char*>(*DefaultFontPath),
				FImGuiContext::DefaultFontSize,
				nullptr,
				GlyphRanges.IsEmpty() ? nullptr : GlyphRanges.GetData());
			if (DefaultFont == nullptr)
			{
				DefaultFont = IO.Fonts->AddFontDefault();
			}

			if (FontPath != nullptr)
			{
				SelectedFont = IO.Fonts->AddFontFromFileTTF(
					reinterpret_cast<const char*>(**FontPath),
					FontSettings.GetSize(),
					nullptr,
					GlyphRanges.IsEmpty() ? nullptr : GlyphRanges.GetData());
			}
			else
			{
				SelectedFont = nullptr;
			}

			uint8* TextureData;
			int32 TextureWidth, TextureHeight, BytesPerPixel;
			IO.Fonts->GetTexDataAsRGBA32(&TextureData, &TextureWidth, &TextureHeight, &BytesPerPixel);

#if WITH_ENGINE
			const FImageView TextureView(TextureData, TextureWidth, TextureHeight, ERawImageFormat::BGRA8);
			FontAtlasTexturePtr.Reset(FImageUtils::CreateTexture2DFromImage(TextureView));
#else
			FontAtlasTexturePtr = FSlateDynamicImageBrush::CreateWithImageData(
				TEXT("ImGuiFontAtlas"),
				FVector2D(TextureWidth, TextureHeight),
				TArray(TextureDataRaw, TextureWidth * TextureHeight * BytesPerPixel));
#endif

			IO.Fonts->SetTexID(FontAtlasTexturePtr.Get());

			Promise->SetValue(SelectedFont != nullptr);

			ImGuiContext->OnPreFrame.Remove(SetSelectedFontDelegateHandle);
			SetSelectedFontDelegateHandle.Reset();
		});

	return Promise->GetFuture();
}
