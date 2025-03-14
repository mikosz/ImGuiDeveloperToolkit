#include "ImGuiDeveloperToolkit/PropertyInspector.h"

#include "Containers/AnsiString.h"
#include "imgui.h"

namespace ImGuiDeveloperToolkit::PropertyInspector
{

namespace Private
{

template <class T, class Enable = void>
constexpr bool TIsNumericPropertyV = false;

template <class T>
constexpr bool TIsNumericPropertyV<T, std::enable_if_t<std::is_base_of_v<TProperty_Numeric<typename T::TCppType>, T>>> =
	true;

static_assert(!TIsNumericPropertyV<int>);
static_assert(TIsNumericPropertyV<FIntProperty>);
static_assert(TIsNumericPropertyV<TProperty_Numeric<int32>>);

template <class T>
struct TImGuiScalarInfo
{
};

template <>
struct TImGuiScalarInfo<int8>
{
	static constexpr ImGuiDataType_ DataType = ImGuiDataType_S8;
	static const char* const Format;
};

template <>
struct TImGuiScalarInfo<uint8>
{
	static constexpr ImGuiDataType_ DataType = ImGuiDataType_U8;
	static const char* const Format;
};

template <>
struct TImGuiScalarInfo<int16>
{
	static constexpr ImGuiDataType_ DataType = ImGuiDataType_S16;
	static const char* const Format;
};

template <>
struct TImGuiScalarInfo<uint16>
{
	static constexpr ImGuiDataType_ DataType = ImGuiDataType_U16;
	static const char* const Format;
};

template <>
struct TImGuiScalarInfo<int32>
{
	static constexpr ImGuiDataType_ DataType = ImGuiDataType_S32;
	static const char* const Format;
};

template <>
struct TImGuiScalarInfo<uint32>
{
	static constexpr ImGuiDataType_ DataType = ImGuiDataType_U32;
	static const char* const Format;
};

template <>
struct TImGuiScalarInfo<int64>
{
	static constexpr ImGuiDataType_ DataType = ImGuiDataType_S64;
	static const char* const Format;
};

template <>
struct TImGuiScalarInfo<uint64>
{
	static constexpr ImGuiDataType_ DataType = ImGuiDataType_U64;
	static const char* const Format;
};

template <>
struct TImGuiScalarInfo<float>
{
	static constexpr ImGuiDataType_ DataType = ImGuiDataType_Float;
	static const char* const Format;
};

template <>
struct TImGuiScalarInfo<double>
{
	static constexpr ImGuiDataType_ DataType = ImGuiDataType_Double;
	static const char* const Format;
};

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

template <class T>
constexpr ImGuiDataType_ TImGuiScalarDataType_V = TImGuiScalarInfo<T>::DataType;

template <class T>
void Inspect(const char* Label, FProperty& Property, T* Outer, TCopyConstType<T, UObject>* OuterObject);
template <class T>
void Inspect(const char* Label, const UStruct& Struct, T* Instance, TCopyConstType<T, UObject>* OuterObject);

template <class T, class OuterType, class Enable = void>
struct FTryInspect;

template <class T, class OuterType>
struct FTryInspect<T, OuterType, std::enable_if_t<TIsNumericPropertyV<T>>>
{
	bool operator()(
		const char* Label, FProperty& Property, OuterType* Outer, TCopyConstType<OuterType, UObject>* OuterObject) const
	{
		T* const NumericProperty = ExactCastField<T>(&Property);
		if (!NumericProperty)
		{
			return false;
		}

		using TCppType = typename T::TCppType;

		ImGui::TableNextRow();

		ImGui::TableNextColumn();
		ImGui::Text("%s", Label);

		ImGui::TableNextColumn();

		constexpr bool bIsConst = TIsConst<OuterType>::Value;

		auto* Ptr = NumericProperty->template ContainerPtrToValuePtr<TCopyConstType<OuterType, TCppType>>(Outer);
		TCppType Value = NumericProperty->GetPropertyValue(Ptr);
		const TCppType Step = 1;
		const TCppType StepFast = 100;
		ImGui::PushID(Label);
		if (ImGui::InputScalar(
				"",
				TImGuiScalarDataType_V<TCppType>,
				&Value,
				&Step,
				&StepFast,
				TImGuiScalarInfo<TCppType>::Format,
				bIsConst ? ImGuiInputTextFlags_ReadOnly : 0))
		{
			if constexpr (!bIsConst)
			{
				NumericProperty->SetPropertyValue(Ptr, Value);
				if (IsValid(OuterObject))
				{
					FPropertyChangedEvent PropertyChangedEvent{NumericProperty, EPropertyChangeType::ValueSet};
					OuterObject->PostEditChangeProperty(PropertyChangedEvent);
				}
			}
		}
		ImGui::PopID();

		return true;
	}
};

template <class OuterType>
struct FTryInspect<FArrayProperty, OuterType>
{
	bool operator()(
		const char* Label, FProperty& Property, OuterType* Outer, TCopyConstType<OuterType, UObject>* OuterObject) const
	{
		FArrayProperty* const ArrayProperty = ExactCastField<FArrayProperty>(&Property);
		if (!ArrayProperty)
		{
			return false;
		}

		auto* ArrayData = ArrayProperty->ContainerPtrToValuePtr<TCopyConstType<OuterType, void>>(Outer);

		if (ArrayData == nullptr || ArrayProperty->Inner == nullptr)
		{
			return true;
		}

		FScriptArrayHelper ArrayHelper{ArrayProperty, ArrayData};

		const int32 NumElements = ArrayHelper.Num();

		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		const bool bShowElements = ImGui::TreeNodeEx(Label, ImGuiTreeNodeFlags_SpanAllColumns);
		ImGui::TableNextColumn();
		ImGui::Text("[%d]", NumElements);

		if (!bShowElements)
		{
			return true;
		}

		ImGui::PushID(Label);

		for (int32 Index = 0; Index < NumElements; ++Index)
		{
			FAnsiStringBuilderBase LabelBuilder;
			LabelBuilder.Append("[").Append(FAnsiString::FromInt(Index)).Append("]");

			ImGui::PushID(Index);
			TCopyConstType<OuterType, void>* RawElementPtr = ArrayHelper.GetRawPtr(Index);
			Inspect(LabelBuilder.ToString(), *ArrayProperty->Inner, RawElementPtr, OuterObject);
			ImGui::PopID();
		}

		ImGui::PopID();

		ImGui::TreePop();

		return true;
	}
};

template <class T>
void Inspect(const char* Label, FStructProperty& StructProperty, T* Outer, TCopyConstType<T, UObject>* OuterObject)
{
	if (!IsValid(StructProperty.Struct))
	{
		return;
	}

	Inspect(
		Label,
		*StructProperty.Struct,
		StructProperty.ContainerPtrToValuePtr<TCopyConstType<T, void>>(Outer),
		OuterObject);
}

template <class T>
void Inspect(const char* Label, FProperty& Property, T* Outer, TCopyConstType<T, UObject>* OuterObject)
{
	if (FTryInspect<FArrayProperty, T>{}(Label, Property, Outer, OuterObject)
		|| FTryInspect<FIntProperty, T>{}(Label, Property, Outer, OuterObject)
		|| FTryInspect<FFloatProperty, T>{}(Label, Property, Outer, OuterObject)
		|| FTryInspect<FDoubleProperty, T>{}(Label, Property, Outer, OuterObject))
	{
		return;
	}

	if (FStructProperty* const StructProperty = CastField<FStructProperty>(&Property))
	{
		Inspect(Label, *StructProperty, Outer, OuterObject);
	}
}

template <class T>
void Inspect(const char* Label, const UStruct& Struct, T* Instance, TCopyConstType<T, UObject>* OuterObject)
{
	ImGui::TableNextRow();
	ImGui::TableNextColumn();
	const bool bShowMembers = ImGui::TreeNodeEx(Label, ImGuiTreeNodeFlags_SpanAllColumns);
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
		FProperty* const Property = *It;

		if (!Property)
		{
			continue;
		}

		const char* const PropertyName =
			reinterpret_cast<const char*>(StringCast<UTF8CHAR>(*Property->GetName()).Get());
		const char* const DisplayName =
			reinterpret_cast<const char*>(StringCast<UTF8CHAR>(*Property->GetDisplayNameText().ToString()).Get());

		ImGui::PushID(PropertyName);
		Inspect(DisplayName, *Property, Instance, OuterObject);
		ImGui::PopID();
	}

	ImGui::TreePop();
}

template <class T>
void InspectObject(const char* Label, const UStruct& Struct, T* Instance, TCopyConstType<T, UObject>* OuterObject)
{
	if (Instance == nullptr)
	{
		return;
	}

	constexpr int TableFlags = ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_Resizable
							   | ImGuiTableFlags_RowBg | ImGuiTableFlags_NoBordersInBody;
	if (ImGui::BeginTable(Label, 2, TableFlags))
	{
		ImGui::TableSetupColumn("Key");
		ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

		Inspect(Label, Struct, Instance, OuterObject);

		ImGui::EndTable();
	}
}

}  // namespace Private

void Inspect(const char* Label, const UStruct& Struct, void* Instance, UObject* OuterObject)
{
	Private::InspectObject(Label, Struct, Instance, OuterObject);
}

void Inspect(const char* Label, const UStruct& Struct, const void* Instance, const UObject* OuterObject)
{
	Private::InspectObject(Label, Struct, Instance, OuterObject);
}

void Inspect(const char* Label, const UClass& Class, UObject& Instance)
{
	Private::InspectObject(Label, Class, &Instance, &Instance);
}

void Inspect(const char* Label, const UClass& Class, const UObject& Instance)
{
	Private::InspectObject(Label, Class, &Instance, &Instance);
}

}  // namespace ImGuiDeveloperToolkit::PropertyInspector
