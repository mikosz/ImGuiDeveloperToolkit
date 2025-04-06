#include "ImGuiDeveloperToolkit/AutoWidget.h"

#include "ImGuiDeveloperToolkit/Private/TypeTraits.h"

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

template <class T UE_REQUIRES(std::is_same_v<std::decay_t<T>, int64>)>
bool AutoWidget(const char* Label, const UEnum& Enum, T& EnumValue)
{
	bool bResult = false;

	const FText PreviewDisplayName = Enum.IsValidEnumValue(EnumValue)
										 ? Enum.GetDisplayNameTextByValue(EnumValue)
										 : LOCTEXT("DefaultEnumPreviewValue", "Select value");
	const FUtf8String PreviewDisplayNameUtf8 = FUtf8String{StringCast<UTF8CHAR>(*PreviewDisplayName.ToString())};

	constexpr bool bIsConst = ImGuiDeveloperToolkit::Private::TValueTypeIsConst_v<T>;

	ImGui::BeginDisabled(bIsConst);

	if (ImGui::BeginCombo(Label, reinterpret_cast<const char*>(*PreviewDisplayNameUtf8)))
	{
		const int32 NumEnums = Enum.NumEnums() - (Enum.ContainsExistingMax() ? 1 : 0);
		for (int32 Index = 0; Index < NumEnums; ++Index)
		{
			const int64 Value = Enum.GetValueByIndex(Index);
			const FText DisplayName = Enum.GetDisplayNameTextByIndex(Index);
			const FUtf8String DisplayNameUtf8{StringCast<UTF8CHAR>(*DisplayName.ToString())};

			if (ImGui::Selectable(reinterpret_cast<const char*>(*DisplayNameUtf8), EnumValue == Value))
			{
				if constexpr (!bIsConst)
				{
					EnumValue = Value;
					bResult = true;
				}
			}

			if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
			{
				if (const FText ToolTip = Enum.GetToolTipTextByIndex(Index); !ToolTip.IsEmpty())
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
						ImVec4{.7f, .7f, .7f, 1.f}, "%s: ", reinterpret_cast<const char*>(*PreviewDisplayNameUtf8));
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

bool AutoWidget(const char* Label, const UEnum& Enum, int64& EnumValue)
{
	return Private::AutoWidget(Label, Enum, EnumValue);
}

bool AutoWidget(const char* Label, const UEnum& Enum, const int64& EnumValue)
{
	return Private::AutoWidget(Label, Enum, EnumValue);
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

}  // namespace ImGuiDeveloperToolkit::Widgets

#undef LOCTEXT_NAMESPACE
