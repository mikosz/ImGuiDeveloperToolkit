#include "ImGuiDeveloperToolkit/ImGuiDeveloperToolkitConfiguration.h"

#include "ImGuiContext.h"
#include "ImGuiDeveloperToolkit/AutoWidget.h"
#include "ImGuiDeveloperToolkit/ImGuiDeveloperToolkitSubsystem.h"
#include "ImGuiDeveloperToolkit/ImGuiDeveloperToolkitWindow.h"
#include "ImGuiDeveloperToolkit/Private/EnumValueRange.h"
#include "ImGuiDeveloperToolkitSettings.h"
#include "ImageUtils.h"
#include "SkeletalRenderPublic.h"

struct ImGuiContextHook;
struct ImGuiSettingsHandler;

namespace ImGuiDeveloperToolkitConfigurationPrivate
{

struct FConfigurationData
{
	FUtf8String FontName = {};
	int32 FontSize = -1;
	FAnsiString Glyphs;
	TMap<FAnsiString, bool> ToolShownByName = {};
};

enum class EConfigurationSection : intptr_t
{
	Font = 1,
	Tools,
};

const FAnsiStringView FontSectionName{"Font"};
const FAnsiStringView ToolsSectionName{"Tools"};

FConfigurationData* GConfigurationData = nullptr;

void* ConfigurationHandler_ReadOpen(ImGuiContext* Ctx, ImGuiSettingsHandler* Handler, const char* Name)
{
	const FAnsiStringView NameView{Name};

	if (NameView == FontSectionName)
	{
		return reinterpret_cast<void*>(EConfigurationSection::Font);
	}

	if (NameView == ToolsSectionName)
	{
		return reinterpret_cast<void*>(EConfigurationSection::Tools);
	}

	return nullptr;
}

void ConfigurationHandler_ReadLine(ImGuiContext*, ImGuiSettingsHandler*, void* Entry, const char* Line)
{
	using namespace ImGuiDeveloperToolkit;

	if (GConfigurationData == nullptr)
	{
		return;
	}

	const FUtf8StringView LineView{Line};

	int32 EqIdx = -1;
	if (!LineView.FindChar(UTF8CHAR{'='}, EqIdx))
	{
		return;
	}

	const FUtf8StringView Key = LineView.Left(EqIdx).TrimStartAndEnd();
	const FUtf8StringView Value = LineView.RightChop(EqIdx + 1).TrimStartAndEnd();

	const EConfigurationSection Section = static_cast<EConfigurationSection>(reinterpret_cast<intptr_t>(Entry));

	if (Section == EConfigurationSection::Font)
	{
		if (Key == FUtf8StringView{"Name"})
		{
			GConfigurationData->FontName = Value;
		}
		else if (Key == FUtf8StringView{"Size"})
		{
			const FUtf8String ValueStr{Value};
			GConfigurationData->FontSize = FCStringUtf8::Atoi(*ValueStr);
		}
		else if (Key == FUtf8StringView{"Glyphs"})
		{
			GConfigurationData->Glyphs = FAnsiString{Value};
		}
	}
	else if (Section == EConfigurationSection::Tools)
	{
		const FUtf8String ValueStr{Value};
		const bool bShown = FCStringUtf8::ToBool(*ValueStr);
		GConfigurationData->ToolShownByName.Emplace(Key, bShown);
	}
}

static void ConfigurationHandler_ApplyAll(ImGuiContext* Ctx, ImGuiSettingsHandler*)
{
	if (GConfigurationData == nullptr || !IsValid(GEngine))
	{
		return;
	}

	UImGuiDeveloperToolkitSubsystem* Subsystem = GEngine->GetEngineSubsystem<UImGuiDeveloperToolkitSubsystem>();
	if (!IsValid(Subsystem))
	{
		return;
	}

	const int32 GlyphRanges = []
	{
		const UEnum* Enum = StaticEnum<EImGuiDeveloperToolkitGlyphRanges>();
		if (!ensure(IsValid(Enum)))
		{
			return static_cast<int32>(EImGuiDeveloperToolkitGlyphRanges::Default);
		}

		TArray<FAnsiString> GlyphNames;
		FAnsiString{GConfigurationData->Glyphs}.ParseIntoArray(GlyphNames, "|");

		int32 Result = 0;
		for (const FAnsiString& GlyphName : GlyphNames)
		{
			Result |= Enum->GetValueByNameString(FString{GlyphName}, EGetByNameFlags::ErrorIfNotFound);
		}

		return Result;
	}();

	UImGuiDeveloperToolkitUserSettings* const Settings = GetMutableDefault<UImGuiDeveloperToolkitUserSettings>();
	if (!IsValid(Settings))
	{
		return;
	}

	Subsystem->Configuration.SetFont(GConfigurationData->FontName, GConfigurationData->FontSize, GlyphRanges);
}

// #TODO_dontcommit: a bunch of utf8 <-> Fstring casts in this file that we don't want

template <class StringType>
StringType JoinGlyphsAsString(const EImGuiDeveloperToolkitGlyphRanges Mask, const StringType& Separator)
{
	using namespace ImGuiDeveloperToolkit::Private;

	return StringType::JoinBy(
		TMaskEnumValueRange<EImGuiDeveloperToolkitGlyphRanges>{Mask},
		*Separator,
		&TEnumValue<EImGuiDeveloperToolkitGlyphRanges>::GetName);
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
static void ConfigurationHandler_WriteAll(ImGuiContext* Ctx, ImGuiSettingsHandler* Handler, ImGuiTextBuffer* Buf)
{
	if (GConfigurationData == nullptr || GConfigurationData->FontName.IsEmpty() && GConfigurationData->FontSize <= 0)
	{
		return;
	}

	Buf->appendf("[ImGuiDeveloperToolkitConfiguration][Font]\n");
	Buf->appendf("Name=%s\n", *GConfigurationData->FontName);
	Buf->appendf("Size=%d\n", GConfigurationData->FontSize);
	Buf->appendf("Glyphs=%s\n", *GConfigurationData->Glyphs);

	Buf->appendf("\n[ImGuiDeveloperToolkitConfiguration][Tools]\n");
	for (const auto& [ToolName, bShow] : GConfigurationData->ToolShownByName)
	{
		Buf->appendf("%s=%d\n", *ToolName, bShow);
	}
}

static void ContextHook_Shutdown(ImGuiContext* Ctx, ImGuiContextHook* Hook)
{
	if (GConfigurationData == nullptr)
	{
		return;
	}

	delete GConfigurationData;
	GConfigurationData = nullptr;
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

FImGuiDeveloperToolkitFontConfiguration::FImGuiDeveloperToolkitFontConfiguration(
	FUtf8String InName, const int32 InSize, const int32 InGlyphRanges)
	: Name{MoveTemp(InName)}, Size{InSize}, GlyphRanges{InGlyphRanges}
{
}

void FImGuiDeveloperToolkitConfiguration::Initialize()
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

	// #TODO_dontcommit: remove or restore
	// FImGuiContext::GetOnPostCreateContext().AddLambda(
	// 	[](ImGuiContext* Context)
	// 	{
	// 		if (Context == nullptr)
	// 		{
	// 			return;
	// 		}
	//
	// 		GConfigurationData = new FConfigurationData;
	//
	// 		// Add .ini handler for stored configuration data
	// 		ImGuiSettingsHandler IniHandler;
	// 		IniHandler.TypeName = "ImGuiDeveloperToolkitConfiguration";
	// 		IniHandler.TypeHash = ImHashStr("ImGuiDeveloperToolkitConfiguration");
	// 		IniHandler.ReadOpenFn = ConfigurationHandler_ReadOpen;
	// 		IniHandler.ReadLineFn = ConfigurationHandler_ReadLine;
	// 		IniHandler.ApplyAllFn = ConfigurationHandler_ApplyAll;
	// 		IniHandler.WriteAllFn = ConfigurationHandler_WriteAll;
	// 		ImGui::AddSettingsHandler(&IniHandler);
	//
	// 		// Add context hook for imgui shutdown
	// 		ImGuiContextHook Hook;
	// 		Hook.Type = ImGuiContextHookType_Shutdown;
	// 		Hook.Callback = &ContextHook_Shutdown;
	// 		ImGui::AddContextHook(Context, &Hook);
	// 	});
}

void FImGuiDeveloperToolkitConfiguration::Tick(const float DeltaTime)
{
	using namespace ImGuiDeveloperToolkit;

	if (bShown)
	{
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
}

void FImGuiDeveloperToolkitConfiguration::SetFont(const FUtf8String& Name, const int32 Size, const int32 GlyphRanges)
{
	FImGuiDeveloperToolkitFontConfiguration FontConfiguration{Name, Size, GlyphRanges};

	if (SettingsType == EImGuiDeveloperToolkitSettingsType::User)
	{
		UImGuiDeveloperToolkitUserSettings::Get().SetFontSettings(MoveTemp(FontConfiguration));
	}
	else
	{
		UImGuiDeveloperToolkitDefaultSettings::Get().FontSettings = MoveTemp(FontConfiguration);
	}

	LoadFonts();
}

ImFont* FImGuiDeveloperToolkitConfiguration::GetFont() const
{
	return SelectedFont;
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void FImGuiDeveloperToolkitConfiguration::SetShown(const FAnsiString& ToolName, const bool bToolShown)
{
	using namespace ImGuiDeveloperToolkitConfigurationPrivate;

	if (GConfigurationData == nullptr)
	{
		return;
	}

	GConfigurationData->ToolShownByName.FindOrAdd(ToolName) = bToolShown;
}

// ReSharper disable once CppMemberFunctionMayBeStatic
bool FImGuiDeveloperToolkitConfiguration::IsShown(const FAnsiString& ToolName) const
{
	using namespace ImGuiDeveloperToolkitConfigurationPrivate;

	if (GConfigurationData == nullptr)
	{
		return false;
	}

	return GConfigurationData->ToolShownByName.FindOrAdd(ToolName, false);
}

void FImGuiDeveloperToolkitConfiguration::TickFontSelector(const float DeltaTime)
{
	ImGui::BeginDisabled(!bFontControlsEnabled);

	const UEnum* const SettingsTypeEnum = StaticEnum<EImGuiDeveloperToolkitSettingsType>();
	check(IsValid(SettingsTypeEnum));
	if (int64 SettingsTypeEnumValue = static_cast<int64>(SettingsType);
		ImGuiDeveloperToolkit::Widgets::AutoWidget("Modified settings", *SettingsTypeEnum, SettingsTypeEnumValue))
	{
		SettingsType = static_cast<EImGuiDeveloperToolkitSettingsType>(SettingsTypeEnumValue);
	}

	const FImGuiDeveloperToolkitFontConfiguration& ReadFontSettings =
		SettingsType == EImGuiDeveloperToolkitSettingsType::User
			? UImGuiDeveloperToolkitUserSettings::Get().GetFontSettings()
			: UImGuiDeveloperToolkitDefaultSettings::Get().FontSettings;
	FImGuiDeveloperToolkitFontConfiguration& WriteFontSettings =
		SettingsType == EImGuiDeveloperToolkitSettingsType::User
			? UImGuiDeveloperToolkitUserSettings::Get().GetUserFontSettings()
			: UImGuiDeveloperToolkitDefaultSettings::Get().FontSettings;

	const bool bFontChanged = [this, &ReadFontSettings, &WriteFontSettings]
	{
		bool bResult = false;

		if (!AvailableFontPathsByName.IsEmpty())
		{
			if (ImGui::BeginCombo(
					"Font",
					ReadFontSettings.Name.IsEmpty() ? "Select font"
													: reinterpret_cast<const char*>(*ReadFontSettings.Name)))
			{
				for (const auto& [Name, Path] : AvailableFontPathsByName)
				{
					if (ImGui::Selectable(reinterpret_cast<const char*>(*Name), ReadFontSettings.Name == FString{Name}))
					{
						bResult = true;
						WriteFontSettings.Name = FString{Name};
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
		static_cast<EImGuiDeveloperToolkitGlyphRanges>(ReadFontSettings.GlyphRanges);
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
		WriteFontSettings.GlyphRanges = static_cast<int32>(GlyphRanges);
	}

	int32 FontSize = ReadFontSettings.Size;
	const bool bSizeChanged = ImGui::SliderInt("Font size", &FontSize, 8, 32);

	if (bSizeChanged)
	{
		WriteFontSettings.Size = FontSize;
	}

	ImGui::EndDisabled();

	if (bFontControlsEnabled && (bFontChanged || bGlyphRangesChanged || bSizeChanged))
	{
		bFontControlsEnabled = false;
		LoadFonts().Next(
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

void FImGuiDeveloperToolkitConfiguration::TickResetFontPopup(const float DeltaTime)
{
	if (ShowResetFontPopup_S > 0.f)
	{
		ImGui::OpenPopup("Keep font?");

		ImGui::PushFont(DefaultFont);

		if (ImGui::BeginPopupModal("Keep font?"))
		{
			ImGui::Text("Example text:");

			ImGui::NewLine();

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
				if (SettingsType == EImGuiDeveloperToolkitSettingsType::User)
				{
					UImGuiDeveloperToolkitUserSettings::Get().GetUserFontSettings() =
						FImGuiDeveloperToolkitFontConfiguration{};
				}
				else
				{
					UImGuiDeveloperToolkitDefaultSettings::Get().ResetFontSettings();
				}
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

TFuture<bool> FImGuiDeveloperToolkitConfiguration::LoadFonts()
{
	using namespace ImGuiDeveloperToolkitConfigurationPrivate;

	const TSharedPtr<TPromise<bool>> Promise = MakeShared<TPromise<bool>>();
	Promise->SetValue(true);
#if 0

	const TSharedPtr<FImGuiContext> ImGuiContext = FImGuiContext::Get(ImGui::GetCurrentContext());
	if (!ImGuiContext.IsValid())
	{
		Promise->SetValue(false);
		return Promise->GetFuture();
	}

	if (GConfigurationData != nullptr)
	{
		GConfigurationData->FontName = FUtf8String{SelectedFontConfiguration.Name};
		GConfigurationData->FontSize = SelectedFontConfiguration.Size;
		GConfigurationData->Glyphs = JoinGlyphsAsString(
			static_cast<EImGuiDeveloperToolkitGlyphRanges>(SelectedFontConfiguration.GlyphRanges), FAnsiString{"|"});
	}

	SetSelectedFontDelegateHandle = ImGuiContext->OnPreFrame.AddLambda(
		[this, ImGuiContext, Promise]
		{
			const ImGuiIO& IO = ImGui::GetIO();

			if (IO.Fonts == nullptr)
			{
				Promise->SetValue(false);
				return;
			}

			const FUtf8String* FontPath = AvailableFontPathsByName.Find(FUtf8String{SelectedFontConfiguration.Name});
			const FUtf8String DefaultFontPath{StringCast<UTF8CHAR>(*FImGuiContext::GetDefaultFontPath())};

			IO.Fonts->Clear();

			TArray<ImWchar, TInlineAllocator<8>> GlyphRanges;
			MakeGlyphRanges(
				GlyphRanges, static_cast<EImGuiDeveloperToolkitGlyphRanges>(SelectedFontConfiguration.GlyphRanges));

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
					SelectedFontConfiguration.Size,
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

#endif
	return Promise->GetFuture();
}
