#include "ImGuiDeveloperToolkit/PropertyInspector.h"

#include "Containers/AnsiString.h"
#include "imgui.h"

#include <type_traits>

namespace ImGuiDeveloperToolkit::PropertyInspector
{

namespace Private
{

template <class From, class To>
using TCopyConstType = std::conditional_t<std::is_const_v<std::remove_reference_t<From>>, std::add_const_t<To>, To>;

static_assert(std::is_same_v<TCopyConstType<const float, int32>, const int32>);
static_assert(std::is_same_v<TCopyConstType<float, int32>, int32>);
static_assert(std::is_same_v<TCopyConstType<const float&, int32>, const int32>);
static_assert(std::is_same_v<TCopyConstType<float&, int32>, int32>);

template <class T>
void Inspect(const char* Label, const FProperty& Property, T* Outer);

template <class T>
void Inspect(const char* Label, const FIntProperty& IntProperty, T* Outer)
{
	ImGui::TableNextRow();

	ImGui::TableNextColumn();
	ImGui::Text("%s", Label);

	ImGui::TableNextColumn();

	constexpr bool bIsConst = TIsConst<T>::Value;

	auto* Ptr = IntProperty.ContainerPtrToValuePtr<TCopyConstType<T, int32>>(Outer);
	int32 Value = IntProperty.GetPropertyValue(Ptr);
	ImGui::PushID(Label);
	if (ImGui::InputInt("", &Value, 1, 100, bIsConst ? ImGuiInputTextFlags_ReadOnly : 0))
	{
		if constexpr (!bIsConst)
		{
			IntProperty.SetPropertyValue(Ptr, Value);
			// make sure property changed gets propagated, maybe even transact?
		}
	}
	ImGui::PopID();
}

template <class T>
void Inspect(const char* Label, const FArrayProperty& ArrayProperty, T* Outer)
{
	auto* ArrayData = ArrayProperty.ContainerPtrToValuePtr<TCopyConstType<T, void>>(Outer);

	if (ArrayData == nullptr || ArrayProperty.Inner == nullptr)
	{
		return;
	}

	FScriptArrayHelper ArrayHelper{&ArrayProperty, ArrayData};

	const int32 NumElements = ArrayHelper.Num();

	ImGui::TableNextRow();
	ImGui::TableNextColumn();
	const bool bShowElements = ImGui::TreeNode(Label);
	ImGui::TableNextColumn();
	ImGui::Text("[%d]", NumElements);

	if (!bShowElements)
	{
		return;
	}

	ImGui::PushID(Label);

	for (int32 Index = 0; Index < NumElements; ++Index)
	{
		FAnsiStringBuilderBase LabelBuilder;
		LabelBuilder.Append("[").Append(FAnsiString::FromInt(Index)).Append("]");

		ImGui::PushID(Index);
		TCopyConstType<T, void>* RawElementPtr = ArrayHelper.GetRawPtr(Index);
		Inspect(LabelBuilder.ToString(), *ArrayProperty.Inner, RawElementPtr);
		ImGui::PopID();
	}

	ImGui::PopID();

	ImGui::TreePop();
}

template <class T>
void Inspect(const char* Label, const UStruct& Struct, T* Instance)
{
	ImGui::TableNextRow();
	ImGui::TableNextColumn();
	const bool bShowMembers = ImGui::TreeNode(Label);
	ImGui::TableNextColumn();

	const char* const TypeDisplayName =
		reinterpret_cast<const char*>(StringCast<UTF8CHAR>(*Struct.GetDisplayNameText().ToString()).Get());
	ImGui::Text("{%s}", TypeDisplayName);

	if (!bShowMembers)
	{
		return;
	}

	for (TFieldIterator<FProperty> It(&Struct); It; ++It)
	{
		const FProperty* const Property = *It;

		if (!Property)
		{
			continue;
		}

		const char* const PropertyName =
			reinterpret_cast<const char*>(StringCast<UTF8CHAR>(*Property->GetName()).Get());
		const char* const DisplayName =
			reinterpret_cast<const char*>(StringCast<UTF8CHAR>(*Property->GetDisplayNameText().ToString()).Get());

		ImGui::PushID(PropertyName);
		Inspect(DisplayName, *Property, Instance);
		ImGui::PopID();
	}

	ImGui::TreePop();
}

template <class T>
void Inspect(const char* Label, const FStructProperty& StructProperty, T* Outer)
{
	if (!IsValid(StructProperty.Struct))
	{
		return;
	}

	Inspect(Label, *StructProperty.Struct, StructProperty.ContainerPtrToValuePtr<TCopyConstType<T, void>>(Outer));
}

template <class T>
void Inspect(const char* Label, const FProperty& Property, T* Outer)
{
	if (const FArrayProperty* const ArrayProperty = CastField<FArrayProperty>(&Property))
	{
		Inspect(Label, *ArrayProperty, Outer);
	}
	if (const FIntProperty* const IntProperty = CastField<FIntProperty>(&Property))
	{
		Inspect(Label, *IntProperty, Outer);
	}
	if (const FStructProperty* const StructProperty = CastField<FStructProperty>(&Property))
	{
		Inspect(Label, *StructProperty, Outer);
	}
}

template <class T>
void InspectObject(const char* Label, const UStruct& Struct, T* Instance)
{
	if (Instance == nullptr)
	{
		return;
	}

	constexpr int TableFlags = ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_Resizable
							   | ImGuiTableFlags_RowBg | ImGuiTableFlags_NoBordersInBody;
	if (ImGui::BeginTable(Label, 2, TableFlags))
	{
		ImGui::TableSetupColumn("Index" /*, ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide*/);
		ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

		Inspect(Label, Struct, Instance);

		ImGui::EndTable();
	}
}

}  // namespace Private

void Inspect(const char* Label, const UStruct& Struct, void* Instance)
{
	Private::InspectObject(Label, Struct, Instance);
}

void Inspect(const char* Label, const UStruct& Struct, const void* Instance)
{
	Private::InspectObject(Label, Struct, Instance);
}

}  // namespace ImGuiDeveloperToolkit::PropertyInspector
