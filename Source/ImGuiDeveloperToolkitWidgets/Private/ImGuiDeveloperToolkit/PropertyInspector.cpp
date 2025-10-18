#include "ImGuiDeveloperToolkit/PropertyInspector.h"

#include "Containers/AnsiString.h"
#include "ImGuiDeveloperToolkit/AutoWidget.h"
#include "ImGuiDeveloperToolkit/Private/ImGuiDeveloperToolkitUtilities.h"
#include "UObject/PropertyAccessUtil.h"
#include "Zakazane/ContinueIfMacros.h"
#include "Zakazane/Math.h"
#include "Zakazane/Property.h"
#include "Zakazane/ReturnIfMacros.h"
#include "Zakazane/TypeTraits.h"
#include "Zakazane/Variant.h"
#include "imgui.h"

namespace ImGuiDeveloperToolkit::PropertyInspector
{

namespace Private
{

constexpr ImGuiTreeNodeFlags DefaultTreeNodeFlags = ImGuiTreeNodeFlags_SpanAllColumns | ImGuiTreeNodeFlags_AllowOverlap;

template <class T, class Enable = void>
constexpr bool TIsNumericPropertyV = false;

template <class T>
constexpr bool TIsNumericPropertyV<T, std::enable_if_t<std::is_base_of_v<TProperty_Numeric<typename T::TCppType>, T>>> =
	true;

static_assert(!TIsNumericPropertyV<int>);
static_assert(TIsNumericPropertyV<FIntProperty>);
static_assert(TIsNumericPropertyV<TProperty_Numeric<int32>>);

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
	const char* ToolTip,
	const UStruct& Struct,
	FPropertyAccessChangeNotify& ChangeNotify,
	T* Instance,
	Zkz::TCopyConstType<T, UObject>* OuterObject,
	const FInspectorSetup& Setup,
	const FInspectorSetup::FTypeSetup& TypeSetup);
template <class T>
void InspectField(
	FProperty& Property,
	FPropertyAccessChangeNotify& ChangeNotify,
	T* Instance,
	Zkz::TCopyConstType<T, UObject>* OuterObject,
	const FInspectorSetup& Setup);

EFieldIterationFlags GetFieldIterationFlags(const FInspectorSetup& Setup, const FInspectorSetup::FTypeSetup& TypeSetup)
{
	EFieldIterationFlags Flags = EFieldIterationFlags::None;

	if (Setup.bIncludeDeprecated)
	{
		Flags |= EFieldIterationFlags::IncludeDeprecated;
	}

	if (!TypeSetup.bShowHierarchy)
	{
		Flags |= EFieldIterationFlags::IncludeSuper;
	}

	return Flags;
}

FInspectorSetup::FTypeSetup GetTypeSetupForBase(const FInspectorSetup::FTypeSetup& ChildTypeSetup)
{
	FInspectorSetup::FTypeSetup Result = ChildTypeSetup;
	Result.bRecurseInto = true;
	return Result;
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

void ShowCompoundTypeRightColumn(const UStruct& Struct)
{
	ImGui::Text("{%s}", IGDT_TEXT_TO_CSTR(Struct.GetDisplayNameText()));
	SetStructItemTooltip(Struct);
}

template <class OuterType>
void ShowCompoundTypeChildren(
	const UStruct& Struct,
	FPropertyAccessChangeNotify& ChangeNotify,
	OuterType* Outer,
	Zkz::TCopyConstType<OuterType, UObject>* OuterObject,
	const FInspectorSetup& Setup,
	const FInspectorSetup::FTypeSetup& TypeSetup)
{
	const auto InspectFields = [&](TFieldIterator<FProperty> FieldIt)
	{
		constexpr int32 NumInlineProperties = 64;
		constexpr int32 NumInlineCategories = 16;
		using FProperties = TArray<FProperty*, TInlineAllocator<NumInlineProperties>>;
		using FPropertiesByCategory = TMap<FString, FProperties, TInlineSetAllocator<NumInlineCategories>>;

		FPropertiesByCategory PropertiesByCategory;

		for (; FieldIt; ++FieldIt)
		{
			FProperty* const Property = *FieldIt;

			ZKZ_CONTINUE_IF(Property == nullptr);
			ZKZ_CONTINUE_IF(
				Setup.OnlyPropertiesMarked != nullptr && !Property->HasMetaData(Setup.OnlyPropertiesMarked));

			static FString EmptyCategory;
			const FString& Category = TypeSetup.bShowCategories ? Property->GetMetaData("Category") : EmptyCategory;

			PropertiesByCategory.FindOrAdd(Category).Emplace(Property);

			ensure(PropertiesByCategory.Num() <= NumInlineCategories);			  // #TODO_dontcommit
			ensure(PropertiesByCategory[Category].Num() <= NumInlineProperties);  // #TODO_dontcommit
		}

		for (const auto& [Category, Properties] : PropertiesByCategory)
		{
			bool bCategoryNodeVisible = false;

			if (TypeSetup.bShowCategories)
			{
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				bCategoryNodeVisible = ImGui::TreeNodeEx(&Category, 0, "[%s]", IGDT_STRING_TO_CSTR(Category));
				ImGui::TableNextColumn();
			}

			if (!TypeSetup.bShowCategories || bCategoryNodeVisible)
			{
				for (FProperty* const Property : Properties)
				{
					InspectField(*Property, ChangeNotify, Outer, OuterObject, Setup);
				}
			}

			if (bCategoryNodeVisible)
			{
				ImGui::TreePop();
			}
		}
	};

	constexpr int32 NumInlineTypes = 8;
	TArray<const UStruct*, TInlineAllocator<NumInlineTypes>> StructHierarchy;

	if (TypeSetup.bShowHierarchy)
	{
		for (const UStruct* CurrentStruct = &Struct;
			 IsValid(CurrentStruct)
			 && (Setup.OnlyChildrenOf == nullptr || CurrentStruct->IsChildOf(Setup.OnlyChildrenOf));
			 CurrentStruct = CurrentStruct->GetSuperStruct())
		{
			StructHierarchy.Emplace(CurrentStruct);
		}

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
				CurrentStruct, DefaultTreeNodeFlags, "{%s}", IGDT_TEXT_TO_CSTR(CurrentStruct->GetDisplayNameText()));
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
			if (TFieldIterator<FProperty> FieldIt{CurrentStruct, GetFieldIterationFlags(Setup, TypeSetup)})
			{
				ImGui::TableNextRow();
				ImGui::TableNextColumn();

				InspectFields(MoveTemp(FieldIt));

				ImGui::TableNextColumn();
			}

			ImGui::TreePop();
		}
	}
	else
	{
		StructHierarchy.Emplace(&Struct);
	}

	ZKZ_RETURN_IF(StructHierarchy.IsEmpty());

	InspectFields(TFieldIterator<FProperty>{StructHierarchy[0], GetFieldIterationFlags(Setup, TypeSetup)});
}

struct FPropertyInspector_Leaf
{
	template <class PropertyType, class OuterType>
	static bool HasChildren(const PropertyType& Property, const OuterType* const Outer, const FInspectorSetup& Setup)
	{
		return false;
	}

	template <class PropertyType, class OuterType>
	static void ShowChildren(
		PropertyType& Property,
		FPropertyAccessChangeNotify& ChangeNotify,
		OuterType* Outer,
		Zkz::TCopyConstType<OuterType, UObject>* OuterObject,
		const FInspectorSetup& Setup)
	{
	}
};

struct FPropertyInspector_Node
{
	template <class PropertyType, class OuterType>
	static bool HasChildren(const PropertyType& Property, const OuterType* const Outer, const FInspectorSetup& Setup)
	{
		return true;
	}
};

template <class InPropertyType>
struct TNumericPropertyInspector : FPropertyInspector_Leaf
{
	using FPropertyType = InPropertyType;

	template <class OuterType>
	static void ShowRightColumn(
		const FPropertyType& Property,
		FPropertyAccessChangeNotify& ChangeNotify,
		OuterType* Outer,
		Zkz::TCopyConstType<OuterType, UObject>* OuterObject,
		const FInspectorSetup& Setup)
	{
		using TCppType = FPropertyType::TCppType;

		constexpr bool bIsConst = TIsConst<OuterType>::Value;

		auto* Ptr = Property.template ContainerPtrToValuePtr<Zkz::TCopyConstType<OuterType, TCppType>>(Outer);
		TCppType Value = Property.GetPropertyValue(Ptr);
		const TCppType OldValue = Value;
		// #TODO_dontcommit: potentially make custom steps per type? If so then move to auto widget. Of a default there
		// per type but allow to override via meta.
		const TCppType Step = 1;
		const TCppType StepFast = 100;

		ImGui::BeginDisabled(bIsConst);
		ImGui::PushID(Ptr);
		if (Widgets::AutoWidget("", Value, Step, StepFast))
		{
			if constexpr (!bIsConst)
			{
				Zkz::EmitPropertyChangeNotifications(
					ChangeNotify,
					Zkz::Math::IsNearlyEqual(OldValue, Value),
					[&] { Property.SetPropertyValue(Ptr, Value); });
			}
		}
		ImGui::PopID();
		ImGui::EndDisabled();
	}
};

struct FStringPropertyInspector : FPropertyInspector_Leaf
{
	using FPropertyType = FStrProperty;

	template <class OuterType>
	static void ShowRightColumn(
		const FPropertyType& Property,
		FPropertyAccessChangeNotify& ChangeNotify,
		OuterType* Outer,
		Zkz::TCopyConstType<OuterType, UObject>* OuterObject,
		const FInspectorSetup& Setup)
	{
		constexpr bool bIsConst = TIsConst<OuterType>::Value;

		auto* Ptr = Property.ContainerPtrToValuePtr<Zkz::TCopyConstType<OuterType, FString>>(Outer);
		const FString& OldValue = FPropertyType::GetPropertyValue(Ptr);
		FString Value = OldValue;

		ImGui::BeginDisabled(bIsConst);
		ImGui::PushID(Ptr);
		if (Widgets::AutoWidget("", Value))
		{
			if constexpr (!bIsConst)
			{
				Zkz::EmitPropertyChangeNotifications(
					ChangeNotify, OldValue == Value, [&] { FPropertyType::SetPropertyValue(Ptr, Value); });
			}
		}
		ImGui::PopID();
		ImGui::EndDisabled();
	}
};

struct FBoolPropertyInspector : FPropertyInspector_Leaf
{
	using FPropertyType = FBoolProperty;

	template <class OuterType>
	static void ShowRightColumn(
		const FPropertyType& Property,
		FPropertyAccessChangeNotify& ChangeNotify,
		OuterType* Outer,
		Zkz::TCopyConstType<OuterType, UObject>* OuterObject,
		const FInspectorSetup& Setup)
	{
		constexpr bool bIsConst = TIsConst<OuterType>::Value;

		auto* Ptr = Property.ContainerPtrToValuePtr<Zkz::TCopyConstType<OuterType, bool>>(Outer);
		const bool OldValue = Property.GetPropertyValue(Ptr);
		bool Value = OldValue;

		ImGui::BeginDisabled(bIsConst);
		ImGui::PushID(Ptr);
		if (Widgets::AutoWidget("", Value))
		{
			if constexpr (!bIsConst)
			{
				Zkz::EmitPropertyChangeNotifications(
					ChangeNotify, OldValue == Value, [&] { Property.SetPropertyValue(Ptr, Value); });
			}
		}
		ImGui::PopID();
		ImGui::EndDisabled();
	}
};

struct FEnumPropertyInspector : FPropertyInspector_Leaf
{
	using FPropertyType = FEnumProperty;

	template <class OuterType>
	static void ShowRightColumn(
		const FPropertyType& Property,
		FPropertyAccessChangeNotify& ChangeNotify,
		OuterType* Outer,
		Zkz::TCopyConstType<OuterType, UObject>* OuterObject,
		const FInspectorSetup& Setup)
	{
		const UEnum* Enum = Property.GetEnum();
		ZKZ_RETURN_IF_INVALID(Enum);

		FNumericProperty* UnderlyingProperty = Property.GetUnderlyingProperty();
		ZKZ_RETURN_IF(!UnderlyingProperty);

		constexpr bool bIsConst = TIsConst<OuterType>::Value;

		auto* Ptr = Property.ContainerPtrToValuePtr<Zkz::TCopyConstType<OuterType, void>>(Outer);
		Zkz::TCopyConstType<OuterType, int64> EnumValue = UnderlyingProperty->GetSignedIntPropertyValue(Ptr);
		const int64 OldValue = EnumValue;

		ImGui::PushID(Ptr);

		if (Widgets::AutoWidget("", *Enum, EnumValue))
		{
			if constexpr (!bIsConst)
			{
				ChangeNotify.ChangeType = EPropertyChangeType::ValueSet;

				Zkz::EmitPropertyChangeNotifications(
					ChangeNotify,
					OldValue == EnumValue,
					[=] { UnderlyingProperty->SetIntPropertyValue(Ptr, EnumValue); });
			}
		}

		ImGui::PopID();
	}
};

struct FArrayPropertyInspector : FPropertyInspector_Node
{
	using FPropertyType = FArrayProperty;

	template <class OuterType>
	static void ShowRightColumn(
		FPropertyType& Property,
		FPropertyAccessChangeNotify& ChangeNotify,
		OuterType* Outer,
		Zkz::TCopyConstType<OuterType, UObject>* OuterObject,
		const FInspectorSetup& Setup)
	{
		auto* ArrayData = Property.ContainerPtrToValuePtr<Zkz::TCopyConstType<OuterType, void>>(Outer);
		ZKZ_RETURN_IF(ArrayData == nullptr || Property.Inner == nullptr);
		FScriptArrayHelper ArrayHelper{&Property, ArrayData};
		const int32 NumElements = ArrayHelper.Num();

		ImGui::Text("[%d]", NumElements);

		ImGui::PushID(ArrayData);
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
			}
		}
	}

	template <class OuterType>
	static void ShowChildren(
		FPropertyType& Property,
		FPropertyAccessChangeNotify& ChangeNotify,
		OuterType* Outer,
		Zkz::TCopyConstType<OuterType, UObject>* OuterObject,
		const FInspectorSetup& Setup)
	{
		auto* ArrayData = Property.ContainerPtrToValuePtr<Zkz::TCopyConstType<OuterType, void>>(Outer);
		ZKZ_RETURN_IF(ArrayData == nullptr || Property.Inner == nullptr);
		FScriptArrayHelper ArrayHelper{&Property, ArrayData};
		const int32 NumElements = ArrayHelper.Num();

		auto* PreviousActiveNode = ChangeNotify.ChangedPropertyChain.GetActiveNode();
		FProperty* PreviousActiveProperty = PreviousActiveNode ? PreviousActiveNode->GetValue() : nullptr;

		ChangeNotify.ChangedPropertyChain.AddHead(Property.Inner);
		ChangeNotify.ChangedPropertyChain.SetActivePropertyNode(&Property);

		ON_SCOPE_EXIT
		{
			ChangeNotify.ChangedPropertyChain.SetActivePropertyNode(PreviousActiveProperty);
			ChangeNotify.ChangedPropertyChain.RemoveNode(Property.Inner);
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
			Inspect(LabelBuilder.ToString(), *Property.Inner, ChangeNotify, RawElementPtr, OuterObject, Setup);

			ImGui::SameLine();
			if (ImGui::SmallButton("-"))
			{
				ArrayHelper.RemoveValues(Index);
				return;
			}
		}
	}
};

struct FStructPropertyInspector : FPropertyInspector_Node
{
	using FPropertyType = FStructProperty;

	template <class OuterType>
	static void ShowRightColumn(
		const FPropertyType& Property,
		FPropertyAccessChangeNotify& ChangeNotify,
		OuterType* Outer,
		Zkz::TCopyConstType<OuterType, UObject>* OuterObject,
		const FInspectorSetup& Setup)
	{
		// For struct properties drop the child parent cutoff (it doesn't make much sense as it won't be in the same
		// type hierarchy).
		const FInspectorSetup StructPropertySetup = [&Setup]
		{
			FInspectorSetup Result = Setup;
			Result.OnlyChildrenOf = nullptr;
			return Setup;
		}();

		ShowCompoundTypeRightColumn(*Property.Struct);
	}

	template <class OuterType>
	static bool HasChildren(const FPropertyType& Property, const OuterType* const Outer, const FInspectorSetup& Setup)
	{
		return Setup.ChildStructureSetup.bRecurseInto;
	}

	template <class OuterType>
	static void ShowChildren(
		FPropertyType& Property,
		FPropertyAccessChangeNotify& ChangeNotify,
		OuterType* Outer,
		Zkz::TCopyConstType<OuterType, UObject>* OuterObject,
		const FInspectorSetup& Setup)
	{
		auto* const StructInstance = Property.ContainerPtrToValuePtr<Zkz::TCopyConstType<OuterType, void>>(Outer);

		ShowCompoundTypeChildren(
			*Property.Struct, ChangeNotify, StructInstance, OuterObject, Setup, Setup.ChildStructureSetup);
	}
};

struct FObjectPropertyInspector : FPropertyInspector_Node
{
	using FPropertyType = FObjectProperty;

	template <class OuterType>
	static bool HasChildren(const FPropertyType& Property, const OuterType* const Outer, const FInspectorSetup& Setup)
	{
		using QualifiedPointerType = Zkz::TCopyConstType<OuterType, UObject>*;
		const QualifiedPointerType Object = Property.GetObjectPropertyValue_InContainer(Outer);

		return Setup.ChildObjectSetup.bRecurseInto && Object != nullptr;
	}

	template <class OuterType>
	static void ShowRightColumn(
		const FPropertyType& Property,
		FPropertyAccessChangeNotify& ChangeNotify,
		OuterType* Outer,
		Zkz::TCopyConstType<OuterType, UObject>* OuterObject,
		const FInspectorSetup& Setup)
	{
		// #TODO #PropertyInspector: would be cool to allow modification of at least the address, but potentially
		// some sort of an object picker?

		using QualifiedPointerType = Zkz::TCopyConstType<OuterType, UObject>*;
		const QualifiedPointerType Object = Property.GetObjectPropertyValue_InContainer(Outer);

		if (!IsValid(Object))
		{
			ImGui::Text("{%s*} nullptr", IGDT_TEXT_TO_CSTR(Property.PropertyClass->GetDisplayNameText()));
			return;
		}

		const UClass* const Class = Object->GetClass();

		ImGui::Text(
			R"({%s*} 0x%p "%s")",
			IGDT_TEXT_TO_CSTR(Class->GetDisplayNameText()),
			static_cast<const void*>(Object),
			IGDT_STRING_TO_CSTR(Object->GetName()));
	}

	template <class OuterType>
	static void ShowChildren(
		FPropertyType& Property,
		FPropertyAccessChangeNotify& ChangeNotify,
		OuterType* Outer,
		Zkz::TCopyConstType<OuterType, UObject>* OuterObject,
		const FInspectorSetup& Setup)
	{
		using QualifiedPointerType = Zkz::TCopyConstType<OuterType, UObject>*;
		const QualifiedPointerType Object = Property.GetObjectPropertyValue_InContainer(Outer);

		ShowCompoundTypeChildren(*Property.PropertyClass, ChangeNotify, Object, Object, Setup, Setup.ChildObjectSetup);
	}
};

using FPropertyInspector = TVariant<
	TNumericPropertyInspector<FByteProperty>,
	TNumericPropertyInspector<FDoubleProperty>,
	TNumericPropertyInspector<FFloatProperty>,
	TNumericPropertyInspector<FInt8Property>,
	TNumericPropertyInspector<FInt64Property>,
	TNumericPropertyInspector<FIntProperty>,
	TNumericPropertyInspector<FUInt16Property>,
	TNumericPropertyInspector<FUInt32Property>,
	TNumericPropertyInspector<FUInt64Property>,
	FStringPropertyInspector,
	FBoolPropertyInspector,
	FEnumPropertyInspector,
	FArrayPropertyInspector,
	FStructPropertyInspector,
	FObjectPropertyInspector>;

template <class OuterType>
void InspectProperty(
	const char* Label,
	FProperty& Property,
	FPropertyAccessChangeNotify& ChangeNotify,
	OuterType* Outer,
	Zkz::TCopyConstType<OuterType, UObject>* OuterObject,
	const FInspectorSetup& Setup)
{
	bool bPropertyMatched = false;

	Zkz::ForEachVariantType<FPropertyInspector>(
		[&]<class VariantType>(Zkz::TTypeTag<VariantType>)
		{
			ZKZ_RETURN_IF(bPropertyMatched);

			using FPropertyType = VariantType::FPropertyType;
			FPropertyType* const VariantProperty = ExactCastField<FPropertyType>(&Property);
			ZKZ_RETURN_IF(!VariantProperty);

			bPropertyMatched = true;

			ImGui::TableNextRow();

			ImGui::TableNextColumn();
			const bool bHasChildren = VariantType::HasChildren(*VariantProperty, Outer, Setup);
			const bool bNodeVisible = ImGui::TreeNodeEx(
				VariantProperty,
				DefaultTreeNodeFlags | (bHasChildren ? ImGuiTreeNodeFlags_None : ImGuiTreeNodeFlags_Leaf),
				"%s",
				Label);
			SetPropertyItemTooltip(Property);

			ImGui::TableNextColumn();
			VariantType::ShowRightColumn(*VariantProperty, ChangeNotify, Outer, OuterObject, Setup);

			ZKZ_RETURN_IF(!bNodeVisible);

			ON_SCOPE_EXIT
			{
				ImGui::TreePop();
			};

			if (bHasChildren)
			{
				VariantType::ShowChildren(*VariantProperty, ChangeNotify, Outer, OuterObject, Setup);
			}
		});

	if (!bPropertyMatched)
	{
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		const bool bNodeVisible =
			ImGui::TreeNodeEx(&Property, DefaultTreeNodeFlags | ImGuiTreeNodeFlags_Leaf, "%s", Label);
		SetPropertyItemTooltip(Property);
		ImGui::TableNextColumn();
		ImGui::Text("Unsupported type: %s", IGDT_TEXT_TO_CSTR(Property.GetClass()->GetDisplayNameText()));
		if (bNodeVisible)
		{
			ImGui::TreePop();
		}
	}
}

template <class T>
void InspectField(
	FProperty& Property,
	FPropertyAccessChangeNotify& ChangeNotify,
	T* Instance,
	Zkz::TCopyConstType<T, UObject>* OuterObject,
	const FInspectorSetup& Setup)
{
	auto* const PreviousActiveNode = ChangeNotify.ChangedPropertyChain.GetActiveNode();
	FProperty* const PreviousActiveProperty = PreviousActiveNode ? PreviousActiveNode->GetValue() : nullptr;

	auto* const PreviousActiveMemberNode = ChangeNotify.ChangedPropertyChain.GetActiveMemberNode();
	FProperty* const PreviousActiveMemberProperty =
		PreviousActiveMemberNode ? PreviousActiveMemberNode->GetValue() : nullptr;

	ChangeNotify.ChangedPropertyChain.AddHead(&Property);
	ChangeNotify.ChangedPropertyChain.SetActivePropertyNode(&Property);
	ChangeNotify.ChangedPropertyChain.SetActiveMemberPropertyNode(&Property);

	ImGui::PushID(&Property);
	Inspect(IGDT_TEXT_TO_CSTR(Property.GetDisplayNameText()), Property, ChangeNotify, Instance, OuterObject, Setup);
	ImGui::PopID();

	ChangeNotify.ChangedPropertyChain.SetActiveMemberPropertyNode(PreviousActiveMemberProperty);
	ChangeNotify.ChangedPropertyChain.SetActivePropertyNode(PreviousActiveProperty);
	ChangeNotify.ChangedPropertyChain.RemoveNode(&Property);
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
	// #TODO_dontcommit inline if works (i.e. eliminate this function and use InspectProperty
	InspectProperty(Label, Property, ChangeNotify, Outer, OuterObject, Setup);
}

/// Inspect function for types - shows a label next to the type's display name. If bRecurseInto is true, also
/// shows properties of that type within an expandable tree node. Note this is used for both classes and structs.
template <class T>
void Inspect(
	const char* Label,
	const char* ToolTip,
	const UStruct& Struct,
	FPropertyAccessChangeNotify& ChangeNotify,
	T* Instance,
	Zkz::TCopyConstType<T, UObject>* OuterObject,
	const FInspectorSetup& Setup,
	const FInspectorSetup::FTypeSetup& TypeSetup)
{
	ImGui::TableNextRow();
	ImGui::TableNextColumn();

	const bool bTreeNodeOpen = ImGui::TreeNodeEx(
		Label, DefaultTreeNodeFlags | (TypeSetup.bRecurseInto ? ImGuiTreeNodeFlags_None : ImGuiTreeNodeFlags_Leaf));
	if (ToolTip != nullptr)
	{
		ImGui::SetItemTooltip("%s", ToolTip);
	}

	ImGui::TableNextColumn();

	ShowCompoundTypeRightColumn(Struct);

	ZKZ_RETURN_IF(!bTreeNodeOpen);

	ON_SCOPE_EXIT
	{
		ImGui::TreePop();
	};

	ZKZ_RETURN_IF(!TypeSetup.bRecurseInto);

	ShowCompoundTypeChildren(Struct, ChangeNotify, Instance, OuterObject, Setup, TypeSetup);
}

template <class T>
void CreateKeyValueTableAndInspect(
	const char* Label,
	const UStruct& Struct,
	FPropertyAccessChangeNotify& ChangeNotify,
	T* Instance,
	Zkz::TCopyConstType<T, UObject>* OuterObject,
	const FInspectorSetup& Setup,
	const FInspectorSetup::FTypeSetup& TypeSetup)
{
	ZKZ_RETURN_IF(Instance == nullptr);

	constexpr int TableFlags = ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_Resizable
							   | ImGuiTableFlags_RowBg | ImGuiTableFlags_NoBordersInBody;
	if (ImGui::BeginTable(Label, 2, TableFlags))
	{
		ImGui::TableSetupColumn("Key");
		ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

		Inspect(Label, nullptr, Struct, ChangeNotify, Instance, OuterObject, Setup, TypeSetup);

		ImGui::EndTable();
	}
}

}  // namespace Private

void Inspect(
	const char* Label, const UStruct& Struct, void* Instance, UObject* OuterObject, const FInspectorSetup& Setup)
{
	FPropertyAccessChangeNotify ChangeNotify;
	ChangeNotify.ChangedObject = OuterObject;

	Private::CreateKeyValueTableAndInspect(
		Label,
		Struct,
		ChangeNotify,
		Instance,
		OuterObject,
		Setup,
		Private::GetTypeSetupForBase(Setup.ChildStructureSetup));
}

void Inspect(
	const char* Label,
	const UStruct& Struct,
	const void* Instance,
	const UObject* OuterObject,
	const FInspectorSetup& Setup)
{
	FPropertyAccessChangeNotify ChangeNotify;

	Private::CreateKeyValueTableAndInspect(
		Label,
		Struct,
		ChangeNotify,
		Instance,
		OuterObject,
		Setup,
		Private::GetTypeSetupForBase(Setup.ChildStructureSetup));
}

void Inspect(const char* Label, const UClass& Class, UObject& Instance, const FInspectorSetup& Setup)
{
	FPropertyAccessChangeNotify ChangeNotify;
	ChangeNotify.ChangedObject = &Instance;

	Private::CreateKeyValueTableAndInspect(
		Label, Class, ChangeNotify, &Instance, &Instance, Setup, Private::GetTypeSetupForBase(Setup.ChildObjectSetup));
}

void Inspect(const char* Label, const UClass& Class, const UObject& Instance, const FInspectorSetup& Setup)
{
	FPropertyAccessChangeNotify ChangeNotify;

	Private::CreateKeyValueTableAndInspect(
		Label, Class, ChangeNotify, &Instance, &Instance, Setup, Private::GetTypeSetupForBase(Setup.ChildObjectSetup));
}

}  // namespace ImGuiDeveloperToolkit::PropertyInspector
