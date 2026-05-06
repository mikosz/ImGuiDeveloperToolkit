#include "ImGuiDeveloperToolkit/AutoWidget.h"

#include "ImGuiDeveloperToolkit/Private/EnumValueRange.h"
#include "ImGuiDeveloperToolkit/Text.h"
#include "Zakazane/TypeTraits.h"

#include <type_traits>

#define LOCTEXT_NAMESPACE "ImGuiDeveloperToolkitAutoWidget"

namespace ImGuiDeveloperToolkit::Widgets
{

namespace Private
{

const char* const TImGuiScalarInfo<int8>::Format = "%hhd";
const char* const TImGuiScalarInfo<uint8>::Format = "%hhu";
const char* const TImGuiScalarInfo<int16>::Format = "%hd";
const char* const TImGuiScalarInfo<uint16>::Format = "%hu";
const char* const TImGuiScalarInfo<int32>::Format = "%d";
const char* const TImGuiScalarInfo<uint32>::Format = "%u";
const char* const TImGuiScalarInfo<int64>::Format = "%lld";
const char* const TImGuiScalarInfo<uint64>::Format = "%llu";
const char* const TImGuiScalarInfo<float>::Format = "%f";
const char* const TImGuiScalarInfo<double>::Format = "%g";

template <class BufType>
struct TImGuiTextCallback
{
	BufType& CharBuf;

	static int InputTextCallback(ImGuiInputTextCallbackData* Data)
	{
		if (!ensure(Data != nullptr && Data->UserData != nullptr))
		{
			return 1;
		}

		const TImGuiTextCallback& This = *static_cast<TImGuiTextCallback*>(Data->UserData);

		switch (Data->EventFlag)
		{
			case ImGuiInputTextFlags_CallbackResize:
				This.CharBuf.SetNum(Data->BufSize);
				break;
			default:
				break;
		}

		return 0;
	}
};

template <class T UE_REQUIRES(std::is_same_v<std::decay_t<T>, int64>)>
bool AutoWidget(const char* Label, const UEnum& Enum, T& EnumValue, EEnumValueType ValueType)
{
	using namespace ImGuiDeveloperToolkit::Private;

	bool bResult = false;

	const FUtf8String PreviewDisplayName = [&Enum, EnumValue, ValueType]
	{
		if (ValueType == EEnumValueType::Mask)
		{
			const FString Preview = FString::JoinBy(
				FMaskEnumValueRange{&Enum, EnumValue},
				TEXT(", "),
				[](const FEnumValue& V) { return V.GetDisplayName().ToString(); });
			return FUtf8String{StringCast<UTF8CHAR>(*Preview)};
		}

		const FText Result = Enum.IsValidEnumValue(EnumValue) ? Enum.GetDisplayNameTextByValue(EnumValue)
															  : LOCTEXT("DefaultEnumPreviewValue", "Select value");
		return FUtf8String{Result.ToString()};
	}();
	constexpr bool bIsConst = Zkz::TValueTypeIsConst_v<T>;

	ImGui::BeginDisabled(bIsConst);

	if (ImGui::BeginCombo(Label, reinterpret_cast<const char*>(*PreviewDisplayName)))
	{
		for (FEnumValueIterator It{&Enum}; It; ++It)
		{
			const int64 Value = It->GetValue();
			const FText DisplayName = It->GetDisplayName();
			const FUtf8String DisplayNameUtf8{StringCast<UTF8CHAR>(*DisplayName.ToString())};

			const bool bSelected = (ValueType == EEnumValueType::Mask ? (EnumValue & Value) != 0 : EnumValue == Value);

			if (ImGui::Selectable(
					reinterpret_cast<const char*>(*DisplayNameUtf8),
					bSelected,
					(ValueType == EEnumValueType::Mask) ? ImGuiSelectableFlags_NoAutoClosePopups : 0))
			{
				if constexpr (!bIsConst)
				{
					EnumValue = ((ValueType == EEnumValueType::Mask) ? (EnumValue ^ Value) : Value);
					bResult = true;
				}
			}

			if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
			{
				if (const FText ToolTip = Enum.GetToolTipTextByIndex(It->GetIndex()); !ToolTip.IsEmpty())
				{
					const FUtf8String ToolTipUtf8{StringCast<UTF8CHAR>(*ToolTip.ToString())};
					ImGui::SetTooltip(reinterpret_cast<const char*>(*ToolTipUtf8));
				}
			}
		}

		ImGui::EndCombo();
	}

#if WITH_EDITOR
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
	{
		const FText ToolTip = Enum.GetToolTipText();
		const FUtf8String ToolTipUtf8{StringCast<UTF8CHAR>(*ToolTip.ToString())};

		const FText PreviewValueToolTip =
			Enum.IsValidEnumValue(EnumValue) ? Enum.GetToolTipTextByIndex(EnumValue) : FText{};
		const FUtf8String PreviewValueToolTipUtf8 = FUtf8String{StringCast<UTF8CHAR>(*PreviewValueToolTip.ToString())};

		if (!ToolTipUtf8.IsEmpty() || !PreviewValueToolTipUtf8.IsEmpty())
		{
			if (ImGui::BeginTooltip())
			{
				if (!ToolTipUtf8.IsEmpty())
				{
					ImGui::Text(reinterpret_cast<const char*>(*ToolTipUtf8));
					if (!PreviewValueToolTipUtf8.IsEmpty())
					{
						ImGui::Separator();
					}
				}

				if (!PreviewValueToolTipUtf8.IsEmpty())
				{
					ImGui::TextColored(
						ImVec4{.7f, .7f, .7f, 1.f}, "%s: ", reinterpret_cast<const char*>(*PreviewDisplayName));
					ImGui::Text(reinterpret_cast<const char*>(*PreviewValueToolTipUtf8));
				}

				ImGui::EndTooltip();
			}
		}
	}
#endif

	ImGui::EndDisabled();

	return bResult;
}

}  // namespace Private

bool AutoWidget(const char* Label, const UEnum& Enum, int64& EnumValue, EEnumValueType ValueType)
{
	return Private::AutoWidget(Label, Enum, EnumValue, ValueType);
}

bool AutoWidget(const char* Label, const UEnum& Enum, const int64& EnumValue, EEnumValueType ValueType)
{
	return Private::AutoWidget(Label, Enum, EnumValue, ValueType);
}

bool AutoWidget(const char* Label, bool& BoolValue)
{
	return ImGui::Checkbox(Label, &BoolValue);
}

bool AutoWidget(const char* Label, const bool& BoolValue)
{
	ImGui::BeginDisabled();

	bool bValue = BoolValue;
	const bool bResult = AutoWidget(Label, bValue);
	ImGui::EndDisabled();

	return bResult;
}

bool AutoWidget(const char* Label, FUtf8String& Utf8StringValue, ImFont* const Font)
{
	const auto FontScopeGuard = PushFont(Font);

	TArray<char, TInlineAllocator<128>> CharBuf;
	const TArray<UTF8CHAR>& UTF8Chars = Utf8StringValue.GetCharArray();
	CharBuf.Reserve(UTF8Chars.Num());
	Algo::Transform(UTF8Chars, CharBuf, [](const UTF8CHAR Char) { return static_cast<char>(Char); });

	Private::TImGuiTextCallback CallbackUserData{CharBuf};
	if (ImGui::InputText(
			Label,
			CharBuf.GetData(),
			CharBuf.Num(),
			ImGuiInputTextFlags_CallbackResize,
			&CallbackUserData.InputTextCallback,
			&CallbackUserData))
	{
		Utf8StringValue = CharBuf.GetData();  // no null terminator for view
		return true;
	}

	return false;
}

bool AutoWidget(const char* Label, const FUtf8StringView Utf8StringValue, ImFont* const Font)
{
	const auto FontScopeGuard = PushFont(Font);

	TArray<char, TInlineAllocator<128>> CharBuf;
	CharBuf.Reserve(Utf8StringValue.NumBytes());
	Algo::Transform(Utf8StringValue, CharBuf, [](const UTF8CHAR Char) { return static_cast<char>(Char); });

	ImGui::BeginDisabled();
	const bool bResult = ImGui::InputText(Label, CharBuf.GetData(), CharBuf.Num());
	ImGui::EndDisabled();

	return bResult;
}

bool AutoWidget(const char* Label, FString& StringValue, ImFont* const Font)
{
	FUtf8String Utf8String{StringValue};
	if (AutoWidget(Label, Utf8String, Font))
	{
		StringValue = FString{Utf8String};
		return true;
	}

	return false;
}

bool AutoWidget(const char* Label, const FString& StringValue, ImFont* const Font)
{
	return AutoWidget(Label, TStringView{StringValue}, Font);
}

}  // namespace ImGuiDeveloperToolkit::Widgets

#undef LOCTEXT_NAMESPACE
