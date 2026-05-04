#include "ImGuiDeveloperToolkit/ImGuiDeveloperToolkitConfigurationWindow.h"

#include "ImGuiContext.h"
#include "ImGuiDeveloperToolkit/AutoWidget.h"
#include "ImGuiDeveloperToolkit/ImGuiDeveloperToolkitWindow.h"
#include "ImGuiDeveloperToolkit/Private/EnumValueRange.h"
#include "ImageUtils.h"
#include "Zakazane/ReturnIfMacros.h"

// #TODO_dontcommit: a bunch of utf8 <-> Fstring casts in this file that we don't want

namespace ImGuiDeveloperToolkitConfigurationPrivate
{

// #TODO_dontcommit check which unused and remove
template <class StringType>
StringType JoinGlyphsAsString(const EImGuiDeveloperToolkitGlyphRanges Mask, const StringType& Separator)
{
	using namespace ImGuiDeveloperToolkit::Private;

	return StringType::JoinBy(
		TMaskEnumValueRange<EImGuiDeveloperToolkitGlyphRanges>{Mask},
		*Separator,
		&TEnumValue<EImGuiDeveloperToolkitGlyphRanges>::GetName);
}

template <class ArrayType>
void MakeGlyphRanges(ArrayType& Ranges, const EImGuiDeveloperToolkitGlyphRanges Mask)
{
	using namespace ImGuiDeveloperToolkit::Private;

	for (TEnumValue EnumValue : TMaskEnumValueRange{Mask})
	{
		switch (EnumValue.GetValue())
		{
			case EImGuiDeveloperToolkitGlyphRanges::BasicLatin:
				Ranges.Append({0x0020, 0x007F});
				break;
			case EImGuiDeveloperToolkitGlyphRanges::Polish:
				Ranges.Append({
					0x0104, 0x0105,	 // Ą, ą
					0x0106, 0x0107,	 // Ć, ć
					0x0118, 0x0119,	 // Ę, ę
					0x0141, 0x0142,	 // Ł, ł
					0x0143, 0x0144,	 // Ń, ń
					0x00D3, 0x00D3,	 // Ó
					0x00F3, 0x00F3,	 // ó
					0x015A, 0x015B,	 // Ś, ś
					0x0179, 0x017A,	 // Ź, ź
					0x017B, 0x017C	 // Ż, ż
				});
				break;
			default:
				ensureMsgf(false, TEXT("Unexpected enum value: %s"), *EnumValue.GetName());
				break;
		}
	}

	if (!Ranges.IsEmpty())
	{
		Ranges.Emplace(0);
	}
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
	ImGui::BeginDisabled(!bFontControlsEnabled);

	UImGuiDeveloperToolkitSettings& SettingsObject = UImGuiDeveloperToolkitSettings::Get();

	FImGuiDeveloperToolkitFontSettings& FontSettings = SettingsObject.FontSettings;

	const bool bFontChanged = [this, &SettingsObject, &FontSettings]
	{
		bool bResult = false;

		if (!AvailableFontPathsByName.IsEmpty())
		{
			if (ImGui::BeginCombo(
					"Font",
					FontSettings.GetName().IsEmpty()
						? "Select font"
						: reinterpret_cast<const char*>(*FUtf8String{FontSettings.GetName()})))
			{
				for (const auto& [Name, Path] : AvailableFontPathsByName)
				{
					if (ImGui::Selectable(
							reinterpret_cast<const char*>(*Name), FontSettings.GetName() == FString{Name}))
					{
						bResult = true;
						FontSettings.SetName(SettingsObject, FString{Name});
					}
				}

				ImGui::EndCombo();
			}
		}

		return bResult;
	}();

	// #TODO_dontcommit: resetting still doesn't work
	// #TODO_dontcommit: can we have mask return whether the mask changed or was the dropdown committed?

	EImGuiDeveloperToolkitGlyphRanges GlyphRanges =
		static_cast<EImGuiDeveloperToolkitGlyphRanges>(FontSettings.GetGlyphRanges());
	const bool bGlyphRangesChanged = [&GlyphRanges]
	{
		const EImGuiDeveloperToolkitGlyphRanges OrigGlyphRanges = GlyphRanges;
		if (ImGuiDeveloperToolkit::Widgets::Mask("Glyph ranges", GlyphRanges))
		{
			return GlyphRanges != OrigGlyphRanges;
		}

		return false;
	}();

	if (bGlyphRangesChanged)
	{
		FontSettings.SetGlyphRanges(SettingsObject, static_cast<int32>(GlyphRanges));
	}

	int32 FontSize = FontSettings.GetSize();
	const bool bSizeChanged = ImGui::SliderInt("Font size", &FontSize, 8, 32);

	if (bSizeChanged)
	{
		FontSettings.SetSize(SettingsObject, FontSize);
	}

	ImGui::EndDisabled();

	if (bFontControlsEnabled && (bFontChanged || bGlyphRangesChanged || bSizeChanged))
	{
		bFontControlsEnabled = false;
		Zkz::IfNotCanceled(
			LoadFonts(),
			[this, bFontChanged, bGlyphRangesChanged](const bool bSuccessful)
			{
				bFontControlsEnabled = true;

				if (!bSuccessful || !(bFontChanged || bGlyphRangesChanged))
				{
					return;
				}

				ShowResetFontPopup_S = 5;
			});
	}

	TickResetFontPopup(DeltaTime);
}

void FImGuiDeveloperToolkitConfigurationWindow::TickResetFontPopup(const float DeltaTime)
{
	if (ShowResetFontPopup_S > 0.f)
	{
		ImGui::OpenPopup("Keep font?");

		ImGui::PushFont(DefaultFont);

		if (ImGui::BeginPopupModal("Keep font?"))
		{
			ImGui::Text("Example text:");

			ImGui::NewLine();

			// #TODO_dontcommit: addexample texts for other charsets
			ImGui::PushFont(SelectedFont);
			ImGui::Text("The quick brown fox jumps over the lazy dog");
			ImGui::PopFont();

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

		ImGui::PopFont();
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

			TArray<ImWchar, TInlineAllocator<8>> GlyphRanges;
			MakeGlyphRanges(GlyphRanges, static_cast<EImGuiDeveloperToolkitGlyphRanges>(FontSettings.GetGlyphRanges()));

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
