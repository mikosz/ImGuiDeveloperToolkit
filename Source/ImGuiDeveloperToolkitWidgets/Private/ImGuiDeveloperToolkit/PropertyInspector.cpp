#include "ImGuiDeveloperToolkit/PropertyInspector.h"

#include "Containers/AnsiString.h"
#include "ImGuiDeveloperToolkit/AutoWidget.h"
#include "ImGuiDeveloperToolkit/Private/ImGuiDeveloperToolkitUtilities.h"
#include "UObject/PropertyAccessUtil.h"
#include "Zakazane/Math.h"
#include "Zakazane/TypeTraits.h"
#include "imgui.h"

namespace ImGuiDeveloperToolkit::PropertyInspector
{

namespace Private
{

EFieldIterationFlags GetFieldIterationFlags(const FInspectorSetup& Setup)
{
	return Setup.bIncludeDeprecated ? EFieldIterationFlags::IncludeDeprecated : EFieldIterationFlags::None;
}

void SetStructItemTooltip(const UStruct& Struct)
{
	if (const FText Tooltip = Struct.GetToolTipText(); !Tooltip.IsEmpty())
	{
		ImGui::SetItemTooltip("%s", IGDT_TEXT_TO_CSTR(Tooltip));
	}
}

void SetPropertyItemTooltip(const FProperty& Property)
{
	if (const FText Tooltip = Property.GetToolTipText(); !Tooltip.IsEmpty())
	{
		ImGui::SetItemTooltip("%s", IGDT_TEXT_TO_CSTR(Tooltip));
	}
}

template <class T, class Enable = void>
constexpr bool TIsNumericPropertyV = false;

template <class T>
constexpr bool TIsNumericPropertyV<T, std::enable_if_t<std::is_base_of_v<TProperty_Numeric<typename T::TCppType>, T>>> =
	true;

static_assert(!TIsNumericPropertyV<int>);
static_assert(TIsNumericPropertyV<FIntProperty>);
static_assert(TIsNumericPropertyV<TProperty_Numeric<int32>>);

template <class ChangeFunctionType>
void EmitPropertyChangeNotifications(
	const FPropertyAccessChangeNotify& ChangeNotify, const bool bIdenticalValue, ChangeFunctionType&& ChangeFunction)
{
	// #TODO_dontcommit: transaction!
	// #TODO_dontcommit: should do the default object copy magic too, probably

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
	TFieldIterator<FProperty> FieldIterator,
	FPropertyAccessChangeNotify& ChangeNotify,
	T* Instance,
	Zkz::TCopyConstType<T, UObject>* OuterObject,
	const FInspectorSetup& Setup);
template <class T>
void Inspect(
	const char* Label,
	FProperty& Property,
	FPropertyAccessChangeNotify& ChangeNotify,
	T* Outer,
	Zkz::TCopyConstType<T, UObject>* OuterObject,
	const FInspectorSetup& Setup);
template <class T>
void Inspect(
	const char* Label,
	FStructProperty& StructProperty,
	FPropertyAccessChangeNotify& ChangeNotify,
	T* Outer,
	Zkz::TCopyConstType<T, UObject>* OuterObject,
	const FInspectorSetup& Setup);
template <class T>
void Inspect(
	const char* Label,
	const UStruct& Struct,
	const bool bRecurseInto,
	FPropertyAccessChangeNotify& ChangeNotify,
	T* Instance,
	Zkz::TCopyConstType<T, UObject>* OuterObject,
	const FInspectorSetup& Setup);

struct FPropertyInspectorBase
{
};

struct FStringPropertyInspector : FPropertyInspectorBase
{
};

struct FBoolPropertyInspector : FPropertyInspectorBase
{
};

template <class T>
struct TNumericPropertyInspector : FPropertyInspectorBase
{
};

struct FEnumPropertyInspector : FPropertyInspectorBase
{
};

struct FArrayPropertyInspector : FPropertyInspectorBase
{
};

struct FStructPropertyInspector : FPropertyInspectorBase
{
};

struct FObjectPropertyInspector : FPropertyInspectorBase
{
};

using FPropertyInspector = TVariant<
	FStringPropertyInspector,
	FBoolPropertyInspector,
	FEnumPropertyInspector,
	FArrayPropertyInspector,
	FStructPropertyInspector,
	FObjectPropertyInspector>;

template <class T, class OuterType, class Enable = void>
struct FTryInspect;

template <class OuterType>
struct FTryInspect<FStrProperty, OuterType>
{
	bool operator()(
		const char* Label,
		FProperty& Property,
		FPropertyAccessChangeNotify& ChangeNotify,
		OuterType* Outer,
		Zkz::TCopyConstType<OuterType, UObject>* OuterObject,
		const FInspectorSetup& Setup) const
	{
		FStrProperty* const StrProperty = ExactCastField<FStrProperty>(&Property);
		if (!StrProperty)
		{
			return false;
		}

		ImGui::TableNextRow();

		ImGui::TableNextColumn();
		ImGui::Text("%s", Label);
		SetPropertyItemTooltip(Property);

		ImGui::TableNextColumn();

		constexpr bool bIsConst = TIsConst<OuterType>::Value;

		auto* Ptr = StrProperty->ContainerPtrToValuePtr<Zkz::TCopyConstType<OuterType, FString>>(Outer);
		const FString& OldValue = StrProperty->GetPropertyValue(Ptr);
		FString Value = OldValue;

		ImGui::BeginDisabled(bIsConst);
		ImGui::PushID(Label);
		if (Widgets::AutoWidget("", Value))
		{
			if constexpr (!bIsConst)
			{
				EmitPropertyChangeNotifications(
					ChangeNotify, OldValue == Value, [=] { StrProperty->SetPropertyValue(Ptr, Value); });
			}
		}
		ImGui::PopID();
		ImGui::EndDisabled();

		return true;
	}
};

template <class OuterType>
struct FTryInspect<FBoolProperty, OuterType>
{
	bool operator()(
		const char* Label,
		FProperty& Property,
		FPropertyAccessChangeNotify& ChangeNotify,
		OuterType* Outer,
		Zkz::TCopyConstType<OuterType, UObject>* OuterObject,
		const FInspectorSetup& Setup) const
	{
		FBoolProperty* const BoolProperty = ExactCastField<FBoolProperty>(&Property);
		if (!BoolProperty)
		{
			return false;
		}

		ImGui::TableNextRow();

		ImGui::TableNextColumn();
		ImGui::Text("%s", Label);
		SetPropertyItemTooltip(Property);

		ImGui::TableNextColumn();

		constexpr bool bIsConst = TIsConst<OuterType>::Value;

		auto* Ptr = BoolProperty->ContainerPtrToValuePtr<Zkz::TCopyConstType<OuterType, bool>>(Outer);
		const bool OldValue = BoolProperty->GetPropertyValue(Ptr);
		bool Value = OldValue;

		ImGui::BeginDisabled(bIsConst);
		ImGui::PushID(Label);
		if (Widgets::AutoWidget("", Value))
		{
			if constexpr (!bIsConst)
			{
				EmitPropertyChangeNotifications(
					ChangeNotify, OldValue == Value, [=] { BoolProperty->SetPropertyValue(Ptr, Value); });
			}
		}
		ImGui::PopID();
		ImGui::EndDisabled();

		return true;
	}
};

template <class T, class OuterType>
struct FTryInspect<T, OuterType, std::enable_if_t<TIsNumericPropertyV<T>>>
{
	bool operator()(
		const char* Label,
		FProperty& Property,
		FPropertyAccessChangeNotify& ChangeNotify,
		OuterType* Outer,
		Zkz::TCopyConstType<OuterType, UObject>* OuterObject,
		const FInspectorSetup& Setup) const
	{
		T* const NumericProperty = ExactCastField<T>(&Property);
		if (!NumericProperty)
		{
			return false;
		}

		using TCppType = T::TCppType;

		ImGui::TableNextRow();

		ImGui::TableNextColumn();
		ImGui::Text("%s", Label);
		SetPropertyItemTooltip(Property);

		ImGui::TableNextColumn();

		constexpr bool bIsConst = TIsConst<OuterType>::Value;

		auto* Ptr = NumericProperty->template ContainerPtrToValuePtr<Zkz::TCopyConstType<OuterType, TCppType>>(Outer);
		TCppType Value = NumericProperty->GetPropertyValue(Ptr);
		const TCppType OldValue = Value;
		// #TODO_dontcommit: potentially make custom steps per type? If so then move to auto widget. Of a default there
		// per type but allow to override via meta.
		const TCppType Step = 1;
		const TCppType StepFast = 100;

		ImGui::BeginDisabled(bIsConst);
		ImGui::PushID(Label);
		if (Widgets::AutoWidget(Label, Value, Step, StepFast))
		{
			if constexpr (!bIsConst)
			{
				EmitPropertyChangeNotifications(
					ChangeNotify,
					Zkz::Math::IsNearlyEqual(OldValue, Value),
					[=] { NumericProperty->SetPropertyValue(Ptr, Value); });
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
		Zkz::TCopyConstType<OuterType, UObject>* OuterObject,
		const FInspectorSetup& Setup) const
	{
		const FEnumProperty* const EnumProperty = ExactCastField<FEnumProperty>(&Property);
		if (!EnumProperty)
		{
			return false;
		}

		ImGui::TableNextRow();

		ImGui::TableNextColumn();
		ImGui::Text("%s", Label);
		SetPropertyItemTooltip(Property);

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

		auto* Ptr = EnumProperty->ContainerPtrToValuePtr<Zkz::TCopyConstType<OuterType, void>>(Outer);
		Zkz::TCopyConstType<OuterType, int64> EnumValue = UnderlyingProperty->GetSignedIntPropertyValue(Ptr);
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
		Zkz::TCopyConstType<OuterType, UObject>* OuterObject,
		const FInspectorSetup& Setup) const
	{
		FArrayProperty* const ArrayProperty = ExactCastField<FArrayProperty>(&Property);
		if (!ArrayProperty)
		{
			return false;
		}

		auto* ArrayData = ArrayProperty->ContainerPtrToValuePtr<Zkz::TCopyConstType<OuterType, void>>(Outer);

		if (ArrayData == nullptr || ArrayProperty->Inner == nullptr)
		{
			return true;
		}

		FScriptArrayHelper ArrayHelper{ArrayProperty, ArrayData};

		const int32 NumElements = ArrayHelper.Num();

		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		const bool bShowElements = ImGui::TreeNodeEx(Label, ImGuiTreeNodeFlags_SpanFullWidth);
		SetPropertyItemTooltip(Property);
		ImGui::TableNextColumn();
		ImGui::Text("[%d]", NumElements);

		ImGui::PushID(Label);
		ON_SCOPE_EXIT
		{
			ImGui::PopID();
		};

		constexpr bool bIsConst = TIsConst<OuterType>::Value;
		if constexpr (!bIsConst)
		{
			ImGui::SameLine();
			if (ImGui::SmallButton("+"))
			{
				ArrayHelper.AddValue();
			}

			ImGui::SameLine();
			if (ImGui::SmallButton("Clr"))
			{
				// #TODO_dontcommit: also notify property changed on array clear / add / remove
				// #TODO_dontcommit: also need to test whether property chains are setup correctly
				// #TODO_dontcommit: also need to make sure properties are marked dirty for replication and assets are marked as modified
				// at least in editor
				// #TODO_dontcommit: make sure transactions are setup so that undo works in editor
				ArrayHelper.Resize(0);

				// Need to exit early as NumElements is not up-to-date
				if (bShowElements)
				{
					ImGui::TreePop();
				}

				return true;
			}
		}

		if (!bShowElements)
		{
			return true;
		}

		ON_SCOPE_EXIT
		{
			ImGui::TreePop();
		};

		auto* PreviousActiveNode = ChangeNotify.ChangedPropertyChain.GetActiveNode();
		FProperty* PreviousActiveProperty = PreviousActiveNode ? PreviousActiveNode->GetValue() : nullptr;

		ChangeNotify.ChangedPropertyChain.AddHead(ArrayProperty->Inner);
		ChangeNotify.ChangedPropertyChain.SetActivePropertyNode(&Property);

		ON_SCOPE_EXIT
		{
			ChangeNotify.ChangedPropertyChain.SetActivePropertyNode(PreviousActiveProperty);
			ChangeNotify.ChangedPropertyChain.RemoveNode(ArrayProperty->Inner);
		};

		for (int32 Index = 0; Index < NumElements; ++Index)
		{
			FAnsiStringBuilderBase LabelBuilder;
			LabelBuilder.Append("[").Append(FAnsiString::FromInt(Index)).Append("]");

			ImGui::PushID(Index);
			ON_SCOPE_EXIT
			{
				ImGui::PopID();
			};

			Zkz::TCopyConstType<OuterType, void>* RawElementPtr = ArrayHelper.GetRawPtr(Index);
			Inspect(LabelBuilder.ToString(), *ArrayProperty->Inner, ChangeNotify, RawElementPtr, OuterObject, Setup);

			ImGui::SameLine();
			if (ImGui::SmallButton("-"))
			{
				ArrayHelper.RemoveValues(Index);
				return true;
			}
		}

		return true;
	}
};

template <class OuterType>
struct FTryInspect<FStructProperty, OuterType>
{
	bool operator()(
		const char* Label,
		FProperty& Property,
		FPropertyAccessChangeNotify& ChangeNotify,
		OuterType* Outer,
		Zkz::TCopyConstType<OuterType, UObject>* OuterObject,
		const FInspectorSetup& Setup) const
	{
		FStructProperty* const StructProperty = CastField<FStructProperty>(&Property);
		if (!StructProperty)
		{
			return false;
		}

		Inspect(Label, *StructProperty, ChangeNotify, Outer, OuterObject, Setup);

		return true;
	}
};

template <class OuterType>
struct FTryInspect<FObjectProperty, OuterType>
{
	bool operator()(
		const char* Label,
		FProperty& Property,
		FPropertyAccessChangeNotify& ChangeNotify,
		OuterType* Outer,
		Zkz::TCopyConstType<OuterType, UObject>* OuterObject,
		const FInspectorSetup& Setup) const
	{
		// #TODO #PropertyInspector: would be cool to allow modification of at least the address, but potentially
		// some sort of an object picker?
		const FObjectProperty* const ObjectProperty = CastField<FObjectProperty>(&Property);
		if (!ObjectProperty)
		{
			return false;
		}

		ImGui::TableNextRow();
		ImGui::TableNextColumn();

		using QualifiedPointerType = Zkz::TCopyConstType<OuterType, UObject>*;
		QualifiedPointerType Object = ObjectProperty->GetObjectPropertyValue_InContainer(Outer);

		const bool bTreeNodeOpen = ImGui::TreeNodeEx(
			Label,
			ImGuiTreeNodeFlags_SpanFullWidth
				| ((!Setup.bRecurseIntoObjects || Object == nullptr) ? ImGuiTreeNodeFlags_Leaf
																	 : ImGuiTreeNodeFlags_None));
		SetPropertyItemTooltip(Property);

		ON_SCOPE_EXIT
		{
			if (bTreeNodeOpen)
			{
				ImGui::TreePop();
			}
		};

		ImGui::TableNextColumn();

		if (!IsValid(Object))
		{
			ImGui::Text("{%s*} nullptr", IGDT_TEXT_TO_CSTR(ObjectProperty->PropertyClass->GetDisplayNameText()));
			return true;
		}

		const UClass* const Class = Object->GetClass();

		ImGui::Text(
			R"({%s*} 0x%p "%s")",
			IGDT_TEXT_TO_CSTR(Class->GetDisplayNameText()),
			static_cast<const void*>(Object),
			IGDT_STRING_TO_CSTR(Object->GetName()));

		if (bTreeNodeOpen && Setup.bRecurseIntoObjects)
		{
			Inspect(
				Label,
				TFieldIterator<FProperty>{Class, GetFieldIterationFlags(Setup)},
				ChangeNotify,
				Object,
				Object,
				Setup);
		}

		return true;
	}
};

template <class T>
void Inspect(
	const char* Label,
	TFieldIterator<FProperty> FieldIterator,
	FPropertyAccessChangeNotify& ChangeNotify,
	T* Instance,
	Zkz::TCopyConstType<T, UObject>* OuterObject,
	const FInspectorSetup& Setup)
{
	for (; FieldIterator; ++FieldIterator)
	{
		FProperty* const Property = *FieldIterator;

		if (!Property)
		{
			continue;
		}

		if (Setup.OnlyPropertiesMarked != nullptr && !Property->HasMetaData(Setup.OnlyPropertiesMarked))
		{
			continue;
		}

		auto* PreviousActiveNode = ChangeNotify.ChangedPropertyChain.GetActiveNode();
		FProperty* PreviousActiveProperty = PreviousActiveNode ? PreviousActiveNode->GetValue() : nullptr;

		auto* PreviousActiveMemberNode = ChangeNotify.ChangedPropertyChain.GetActiveMemberNode();
		FProperty* PreviousActiveMemberProperty =
			PreviousActiveMemberNode ? PreviousActiveMemberNode->GetValue() : nullptr;

		ChangeNotify.ChangedPropertyChain.AddHead(Property);
		ChangeNotify.ChangedPropertyChain.SetActivePropertyNode(Property);
		ChangeNotify.ChangedPropertyChain.SetActiveMemberPropertyNode(Property);

		ImGui::PushID(Property);
		Inspect(
			IGDT_TEXT_TO_CSTR(Property->GetDisplayNameText()), *Property, ChangeNotify, Instance, OuterObject, Setup);
		ImGui::PopID();

		ChangeNotify.ChangedPropertyChain.SetActiveMemberPropertyNode(PreviousActiveMemberProperty);
		ChangeNotify.ChangedPropertyChain.SetActivePropertyNode(PreviousActiveProperty);
		ChangeNotify.ChangedPropertyChain.RemoveNode(Property);
	}
}

template <class T>
void Inspect(
	const char* Label,
	FStructProperty& StructProperty,
	FPropertyAccessChangeNotify& ChangeNotify,
	T* Outer,
	Zkz::TCopyConstType<T, UObject>* OuterObject,
	const FInspectorSetup& Setup)
{
	if (!IsValid(StructProperty.Struct))
	{
		return;
	}

	// For struct properties drop the child parent cutoff (it doesn't make much sense as it won't be in the same
	// type hierarchy).
	const FInspectorSetup StructPropertySetup = [&Setup]
	{
		FInspectorSetup Result = Setup;
		Result.OnlyChildrenOf = nullptr;
		return Setup;
	}();

	Inspect(
		Label,
		*StructProperty.Struct,
		Setup.bRecurseIntoStructs,
		ChangeNotify,
		StructProperty.ContainerPtrToValuePtr<Zkz::TCopyConstType<T, void>>(Outer),
		OuterObject,
		StructPropertySetup);
}

// #TODO_dontcommit: should expose this as well, probably. This is templated only for the constness, so
// could make two functions for a const outer and a non-const outer.
template <class T>
void Inspect(
	const char* Label,
	FProperty& Property,
	FPropertyAccessChangeNotify& ChangeNotify,
	T* Outer,
	Zkz::TCopyConstType<T, UObject>* OuterObject,
	const FInspectorSetup& Setup)
{
	if (
		// NumericProperty subtypes
		FTryInspect<FByteProperty, T>{}(Label, Property, ChangeNotify, Outer, OuterObject, Setup)
		|| FTryInspect<FDoubleProperty, T>{}(Label, Property, ChangeNotify, Outer, OuterObject, Setup)
		|| FTryInspect<FFloatProperty, T>{}(Label, Property, ChangeNotify, Outer, OuterObject, Setup)
		|| FTryInspect<FInt8Property, T>{}(Label, Property, ChangeNotify, Outer, OuterObject, Setup)
		|| FTryInspect<FInt64Property, T>{}(Label, Property, ChangeNotify, Outer, OuterObject, Setup)
		|| FTryInspect<FIntProperty, T>{}(Label, Property, ChangeNotify, Outer, OuterObject, Setup)
		|| FTryInspect<FUInt16Property, T>{}(Label, Property, ChangeNotify, Outer, OuterObject, Setup)
		|| FTryInspect<FUInt32Property, T>{}(Label, Property, ChangeNotify, Outer, OuterObject, Setup)
		|| FTryInspect<FUInt64Property, T>{}(Label, Property, ChangeNotify, Outer, OuterObject, Setup)
		// Other properties
		|| FTryInspect<FStrProperty, T>{}(Label, Property, ChangeNotify, Outer, OuterObject, Setup)
		|| FTryInspect<FBoolProperty, T>{}(Label, Property, ChangeNotify, Outer, OuterObject, Setup)
		|| FTryInspect<FArrayProperty, T>{}(Label, Property, ChangeNotify, Outer, OuterObject, Setup)
		|| FTryInspect<FEnumProperty, T>{}(Label, Property, ChangeNotify, Outer, OuterObject, Setup)
		|| FTryInspect<FStructProperty, T>{}(Label, Property, ChangeNotify, Outer, OuterObject, Setup)
		|| FTryInspect<FObjectProperty, T>{}(Label, Property, ChangeNotify, Outer, OuterObject, Setup))
	{
		return;
	}

	ImGui::TableNextRow();
	ImGui::TableNextColumn();
	ImGui::Text("%s", Label);
	ImGui::TableNextColumn();
	ImGui::Text("Unsupported type: %s", IGDT_TEXT_TO_CSTR(Property.GetClass()->GetDisplayNameText()));
}

/// Inspect function for types - shows a label next to the type's display name. If bRecurseInto is true, also
/// shows properties of that type within an expandable tree node. Note this is used for both classes and structs.
template <class T>
void Inspect(
	const char* Label,
	const char* ToolTip,
	const UStruct& Struct,
	const bool bRecurseInto,
	FPropertyAccessChangeNotify& ChangeNotify,
	T* Instance,
	Zkz::TCopyConstType<T, UObject>* OuterObject,
	const FInspectorSetup& Setup)
{
	ImGui::TableNextRow();
	ImGui::TableNextColumn();
	const bool bTreeNodeOpen = ImGui::TreeNodeEx(
		Label, ImGuiTreeNodeFlags_SpanFullWidth | (bRecurseInto ? ImGuiTreeNodeFlags_None : ImGuiTreeNodeFlags_Leaf));
	if (ToolTip != nullptr)
	{
		ImGui::SetItemTooltip("%s", ToolTip);
	}
	ImGui::TableNextColumn();

	ImGui::Text("{%s}", IGDT_TEXT_TO_CSTR(Struct.GetDisplayNameText()));
	SetStructItemTooltip(Struct);

	if (!bTreeNodeOpen)
	{
		return;
	}

	ON_SCOPE_EXIT
	{
		ImGui::TreePop();
	};

	if (!bRecurseInto)
	{
		return;
	}

	TArray<const UStruct*, TInlineAllocator<16>> StructHierarchy;

	for (const UStruct* CurrentStruct = &Struct;
		 IsValid(CurrentStruct) && (Setup.OnlyChildrenOf == nullptr || CurrentStruct->IsChildOf(Setup.OnlyChildrenOf));
		 CurrentStruct = CurrentStruct->GetSuperStruct())
	{
		StructHierarchy.Emplace(CurrentStruct);
	}

	const auto VisitStruct = [&](TFieldIterator<FProperty> FieldIt)
	{ Inspect(Label, FieldIt, ChangeNotify, Instance, OuterObject, Setup); };

	int32 NumOpen = 0;
	for (int32 HierarchyIdx = 1; HierarchyIdx < StructHierarchy.Num(); ++HierarchyIdx)
	{
		ImGui::TableNextRow();
		ImGui::TableNextColumn();

		ON_SCOPE_EXIT
		{
			ImGui::TableNextColumn();
		};

		const UStruct* const CurrentStruct = StructHierarchy[HierarchyIdx];
		const bool bNodeOpen = ImGui::TreeNodeEx(
			CurrentStruct, ImGuiTreeNodeFlags_None, "{%s}", IGDT_TEXT_TO_CSTR(CurrentStruct->GetDisplayNameText()));
		SetStructItemTooltip(*CurrentStruct);

		if (!bNodeOpen)
		{
			break;
		}

		++NumOpen;
	}

	for (int32 ParentIdx = NumOpen; ParentIdx > 0; --ParentIdx)
	{
		const UStruct* const CurrentStruct = StructHierarchy[ParentIdx];
		if (TFieldIterator<FProperty> FieldIt{CurrentStruct, GetFieldIterationFlags(Setup)})
		{
			ImGui::TableNextRow();
			ImGui::TableNextColumn();

			VisitStruct(MoveTemp(FieldIt));

			ImGui::TableNextColumn();
		}

		ImGui::TreePop();
	}

	VisitStruct(TFieldIterator<FProperty>{StructHierarchy[0], GetFieldIterationFlags(Setup)});
}

template <class T>
void CreateKeyValueTableAndInspect(
	const char* Label,
	const UStruct& Struct,
	FPropertyAccessChangeNotify& ChangeNotify,
	T* Instance,
	Zkz::TCopyConstType<T, UObject>* OuterObject,
	const FInspectorSetup& Setup)
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

		Inspect(Label, nullptr, Struct, true, ChangeNotify, Instance, OuterObject, Setup);

		ImGui::EndTable();
	}
}

}  // namespace Private

// #TODO_dontcommit replace const char* with FAnsiStringView?
void Inspect(
	const char* Label, const UStruct& Struct, void* Instance, UObject* OuterObject, const FInspectorSetup& Setup)
{
	FPropertyAccessChangeNotify ChangeNotify;
	ChangeNotify.ChangedObject = OuterObject;

	Private::CreateKeyValueTableAndInspect(Label, Struct, ChangeNotify, Instance, OuterObject, Setup);
}

void Inspect(
	const char* Label,
	const UStruct& Struct,
	const void* Instance,
	const UObject* OuterObject,
	const FInspectorSetup& Setup)
{
	FPropertyAccessChangeNotify ChangeNotify;

	Private::CreateKeyValueTableAndInspect(Label, Struct, ChangeNotify, Instance, OuterObject, Setup);
}

void Inspect(const char* Label, const UClass& Class, UObject& Instance, const FInspectorSetup& Setup)
{
	FPropertyAccessChangeNotify ChangeNotify;
	ChangeNotify.ChangedObject = &Instance;

	Private::CreateKeyValueTableAndInspect(Label, Class, ChangeNotify, &Instance, &Instance, Setup);
}

void Inspect(const char* Label, const UClass& Class, const UObject& Instance, const FInspectorSetup& Setup)
{
	FPropertyAccessChangeNotify ChangeNotify;

	Private::CreateKeyValueTableAndInspect(Label, Class, ChangeNotify, &Instance, &Instance, Setup);
}

}  // namespace ImGuiDeveloperToolkit::PropertyInspector
