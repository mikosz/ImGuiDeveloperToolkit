#include "ImGuiDeveloperToolkit/PropertyInspector.h"

#include "Containers/AnsiString.h"
#include "ImGuiDeveloperToolkit/AutoWidget.h"
#include "UObject/PropertyAccessUtil.h"
#include "imgui.h"

#define LOCTEXT_NAMESPACE "ImGuiDeveloperToolkitWidgetsPropertyInspector"

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

template <class ChangeFunctionType>
void EmitPropertyChangeNotifications(
	const FPropertyAccessChangeNotify& ChangeNotify, const bool bIdenticalValue, ChangeFunctionType&& ChangeFunction)
{
	PropertyAccessUtil::EmitPreChangeNotify(&ChangeNotify, bIdenticalValue);
	if (!bIdenticalValue)
	{
		ChangeFunction();
	}
	PropertyAccessUtil::EmitPostChangeNotify(&ChangeNotify, bIdenticalValue);
}

template <class T>
void Inspect(
	const char* Label,
	FProperty& Property,
	FPropertyAccessChangeNotify& ChangeNotify,
	T* Outer,
	ImGuiDeveloperToolkit::Private::TCopyConstType<T, UObject>* OuterObject);
template <class T>
void Inspect(
	const char* Label,
	const UStruct& Struct,
	FPropertyAccessChangeNotify& ChangeNotify,
	T* Instance,
	ImGuiDeveloperToolkit::Private::TCopyConstType<T, UObject>* OuterObject);

template <class T, class OuterType, class Enable = void>
struct FTryInspect;

template <class T, class OuterType>
struct FTryInspect<T, OuterType, std::enable_if_t<TIsNumericPropertyV<T>>>
{
	bool operator()(
		const char* Label,
		FProperty& Property,
		FPropertyAccessChangeNotify& ChangeNotify,
		OuterType* Outer,
		ImGuiDeveloperToolkit::Private::TCopyConstType<OuterType, UObject>* OuterObject) const
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

		auto* Ptr =
			NumericProperty
				->template ContainerPtrToValuePtr<ImGuiDeveloperToolkit::Private::TCopyConstType<OuterType, TCppType>>(
					Outer);
		TCppType Value = NumericProperty->GetPropertyValue(Ptr);
		const TCppType OldValue = Value;
		const TCppType Step = 1;
		const TCppType StepFast = 100;

		ImGui::BeginDisabled(bIsConst);
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
				EmitPropertyChangeNotifications(
					ChangeNotify, OldValue == Value, [=] { NumericProperty->SetPropertyValue(Ptr, Value); });
			}
		}
		ImGui::PopID();
		ImGui::EndDisabled();

		return true;
	}
};

template <class OuterType>
struct FTryInspect<FEnumProperty, OuterType>
{
	bool operator()(
		const char* Label,
		FProperty& Property,
		FPropertyAccessChangeNotify& ChangeNotify,
		OuterType* Outer,
		ImGuiDeveloperToolkit::Private::TCopyConstType<OuterType, UObject>* OuterObject) const
	{
		const FEnumProperty* const EnumProperty = ExactCastField<FEnumProperty>(&Property);
		if (!EnumProperty)
		{
			return false;
		}

		ImGui::TableNextRow();

		ImGui::TableNextColumn();
		ImGui::Text("%s", Label);

		ImGui::TableNextColumn();

		const UEnum* Enum = EnumProperty->GetEnum();
		if (!IsValid(Enum))
		{
			return true;
		}

		FNumericProperty* UnderlyingProperty = EnumProperty->GetUnderlyingProperty();
		if (!UnderlyingProperty)
		{
			return true;
		}

		constexpr bool bIsConst = TIsConst<OuterType>::Value;

		// #TODO_dontcommit the need to say ImGuiDeveloperToolkit::Private:: for type traits is irritating
		auto* Ptr =
			EnumProperty->ContainerPtrToValuePtr<ImGuiDeveloperToolkit::Private::TCopyConstType<OuterType, void>>(
				Outer);
		ImGuiDeveloperToolkit::Private::TCopyConstType<OuterType, int64> EnumValue =
			UnderlyingProperty->GetSignedIntPropertyValue(Ptr);
		const int64 OldValue = EnumValue;

		// #TODO_dontcommit: tooltips everywhere!

		ImGui::PushID(Label);

		if (Widgets::AutoWidget("", *Enum, EnumValue))
		{
			if constexpr (!bIsConst)
			{
				ChangeNotify.ChangeType = EPropertyChangeType::ValueSet;

				EmitPropertyChangeNotifications(
					ChangeNotify,
					OldValue == EnumValue,
					[=] { UnderlyingProperty->SetIntPropertyValue(Ptr, EnumValue); });
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
		const char* Label,
		FProperty& Property,
		FPropertyAccessChangeNotify& ChangeNotify,
		OuterType* Outer,
		ImGuiDeveloperToolkit::Private::TCopyConstType<OuterType, UObject>* OuterObject) const
	{
		FArrayProperty* const ArrayProperty = ExactCastField<FArrayProperty>(&Property);
		if (!ArrayProperty)
		{
			return false;
		}

		auto* ArrayData =
			ArrayProperty->ContainerPtrToValuePtr<ImGuiDeveloperToolkit::Private::TCopyConstType<OuterType, void>>(
				Outer);

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

		auto* PreviousActiveNode = ChangeNotify.ChangedPropertyChain.GetActiveNode();
		FProperty* PreviousActiveProperty = PreviousActiveNode ? PreviousActiveNode->GetValue() : nullptr;

		ChangeNotify.ChangedPropertyChain.AddHead(ArrayProperty->Inner);
		ChangeNotify.ChangedPropertyChain.SetActivePropertyNode(&Property);

		for (int32 Index = 0; Index < NumElements; ++Index)
		{
			FAnsiStringBuilderBase LabelBuilder;
			LabelBuilder.Append("[").Append(FAnsiString::FromInt(Index)).Append("]");

			ImGui::PushID(Index);
			ImGuiDeveloperToolkit::Private::TCopyConstType<OuterType, void>* RawElementPtr =
				ArrayHelper.GetRawPtr(Index);
			Inspect(LabelBuilder.ToString(), *ArrayProperty->Inner, ChangeNotify, RawElementPtr, OuterObject);
			ImGui::PopID();
		}

		ChangeNotify.ChangedPropertyChain.SetActivePropertyNode(PreviousActiveProperty);
		ChangeNotify.ChangedPropertyChain.RemoveNode(ArrayProperty->Inner);

		ImGui::PopID();

		ImGui::TreePop();

		return true;
	}
};

template <class T>
void Inspect(
	const char* Label,
	FStructProperty& StructProperty,
	FPropertyAccessChangeNotify& ChangeNotify,
	T* Outer,
	ImGuiDeveloperToolkit::Private::TCopyConstType<T, UObject>* OuterObject)
{
	if (!IsValid(StructProperty.Struct))
	{
		return;
	}

	Inspect(
		Label,
		*StructProperty.Struct,
		ChangeNotify,
		StructProperty.ContainerPtrToValuePtr<ImGuiDeveloperToolkit::Private::TCopyConstType<T, void>>(Outer),
		OuterObject);
}

template <class T>
void Inspect(
	const char* Label,
	FProperty& Property,
	FPropertyAccessChangeNotify& ChangeNotify,
	T* Outer,
	ImGuiDeveloperToolkit::Private::TCopyConstType<T, UObject>* OuterObject)
{
	if (FTryInspect<FIntProperty, T>{}(Label, Property, ChangeNotify, Outer, OuterObject)
		|| FTryInspect<FFloatProperty, T>{}(Label, Property, ChangeNotify, Outer, OuterObject)
		|| FTryInspect<FDoubleProperty, T>{}(Label, Property, ChangeNotify, Outer, OuterObject)
		|| FTryInspect<FArrayProperty, T>{}(Label, Property, ChangeNotify, Outer, OuterObject)
		|| FTryInspect<FEnumProperty, T>{}(Label, Property, ChangeNotify, Outer, OuterObject))
	{
		return;
	}

	if (FStructProperty* const StructProperty = CastField<FStructProperty>(&Property))
	{
		Inspect(Label, *StructProperty, ChangeNotify, Outer, OuterObject);
	}
}

template <class T>
void Inspect(
	const char* Label,
	const UStruct& Struct,
	FPropertyAccessChangeNotify& ChangeNotify,
	T* Instance,
	ImGuiDeveloperToolkit::Private::TCopyConstType<T, UObject>* OuterObject)
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

		auto* PreviousActiveNode = ChangeNotify.ChangedPropertyChain.GetActiveNode();
		FProperty* PreviousActiveProperty = PreviousActiveNode ? PreviousActiveNode->GetValue() : nullptr;

		auto* PreviousActiveMemberNode = ChangeNotify.ChangedPropertyChain.GetActiveMemberNode();
		FProperty* PreviousActiveMemberProperty =
			PreviousActiveMemberNode ? PreviousActiveMemberNode->GetValue() : nullptr;

		ChangeNotify.ChangedPropertyChain.AddHead(Property);
		ChangeNotify.ChangedPropertyChain.SetActivePropertyNode(Property);
		ChangeNotify.ChangedPropertyChain.SetActiveMemberPropertyNode(Property);

		ImGui::PushID(PropertyName);
		Inspect(DisplayName, *Property, ChangeNotify, Instance, OuterObject);
		ImGui::PopID();

		ChangeNotify.ChangedPropertyChain.SetActiveMemberPropertyNode(PreviousActiveMemberProperty);
		ChangeNotify.ChangedPropertyChain.SetActivePropertyNode(PreviousActiveProperty);
		ChangeNotify.ChangedPropertyChain.RemoveNode(Property);
	}

	ImGui::TreePop();
}

template <class T>
void InspectObject(
	const char* Label,
	const UStruct& Struct,
	FPropertyAccessChangeNotify& ChangeNotify,
	T* Instance,
	ImGuiDeveloperToolkit::Private::TCopyConstType<T, UObject>* OuterObject)
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

		Inspect(Label, Struct, ChangeNotify, Instance, OuterObject);

		ImGui::EndTable();
	}
}

}  // namespace Private

void Inspect(const char* Label, const UStruct& Struct, void* Instance, UObject* OuterObject)
{
	FPropertyAccessChangeNotify ChangeNotify;
	ChangeNotify.ChangedObject = OuterObject;

	Private::InspectObject(Label, Struct, ChangeNotify, Instance, OuterObject);
}

void Inspect(const char* Label, const UStruct& Struct, const void* Instance, const UObject* OuterObject)
{
	FPropertyAccessChangeNotify ChangeNotify;

	Private::InspectObject(Label, Struct, ChangeNotify, Instance, OuterObject);
}

void Inspect(const char* Label, const UClass& Class, UObject& Instance)
{
	FPropertyAccessChangeNotify ChangeNotify;
	ChangeNotify.ChangedObject = &Instance;

	Private::InspectObject(Label, Class, ChangeNotify, &Instance, &Instance);
}

void Inspect(const char* Label, const UClass& Class, const UObject& Instance)
{
	FPropertyAccessChangeNotify ChangeNotify;

	Private::InspectObject(Label, Class, ChangeNotify, &Instance, &Instance);
}

}  // namespace ImGuiDeveloperToolkit::PropertyInspector

#undef LOCTEXT_NAMESPACE
