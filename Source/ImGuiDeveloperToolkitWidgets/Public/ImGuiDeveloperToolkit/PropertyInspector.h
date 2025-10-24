// Copyright 2025 Mikołaj Radwan, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Zakazane/TypeTraits.h"

namespace ImGuiDeveloperToolkit::PropertyInspector
{

struct FInspectorSetup
{
	struct FTypeSetup
	{
		/// Whether to show the contents of child types
		bool bRecurseInto = false;
		
		/// Whether to group inherited properties by superclass 
		bool bShowHierarchy = false;
		
		/// Whether to show property categories
		bool bShowCategories = false;
	};
	
	/// When inspecting UStructs or UClasses will only show properties of this type or a child type. E.g. for UObject
	/// if you inspect a UPrimitiveComponent and set this to UActorComponent, will show properties of UPrimitiveComponent,
	/// USceneComponent, and UActorComponent.
	UStruct* OnlyChildrenOf = nullptr;

	/// Setup for displaying properties of structures
	FTypeSetup StructSetup = {.bRecurseInto = true, .bShowHierarchy = false, .bShowCategories = true};

	/// Setup for displaying properties of pointed-to objects
	FTypeSetup ObjectSetup = {.bRecurseInto = false, .bShowHierarchy = true, .bShowCategories = false};

	/// If set, will only show properties marked with the given meta tag
	const TCHAR* OnlyPropertiesMarked = nullptr;

	/// If set, will only inspect types that satisfy this predicate
	TFunction<bool(const UStruct&)> TypePredicate;

	/// If set, will only inspect properties that satisfy this predicate
	TFunction<bool(const UProperty&)> PropertyPredicate;

	/// Whether to include deprecated properties
	bool bIncludeDeprecated = false;
};

IMGUIDEVELOPERTOOLKITWIDGETS_API void Inspect(
	const char* Label, const UStruct& Struct, void* Instance, UObject* OuterObject, const FInspectorSetup& Setup = {});
IMGUIDEVELOPERTOOLKITWIDGETS_API void Inspect(
	const char* Label,
	const UStruct& Struct,
	const void* Instance,
	const UObject* OuterObject,
	const FInspectorSetup& Setup = {});
IMGUIDEVELOPERTOOLKITWIDGETS_API void Inspect(
	const char* Label, const UClass& Class, UObject& Instance, const FInspectorSetup& Setup = {});
IMGUIDEVELOPERTOOLKITWIDGETS_API void Inspect(
	const char* Label, const UClass& Class, const UObject& Instance, const FInspectorSetup& Setup = {});

template <class T UE_REQUIRES(Zkz::TIsUHTUStruct_v<T>)>
void Inspect(
	const char* Label, T& Instance, Zkz::TCopyConstType<T, UObject>* OuterObject, const FInspectorSetup& Setup = {})
{
	if (const UStruct* Struct = T::StaticStruct(); IsValid(Struct))
	{
		Inspect(Label, *Struct, &Instance, OuterObject, Setup);
	}
}

template <class T UE_REQUIRES(Zkz::TIsUHTUClass_v<T>)>
void Inspect(const char* Label, T& Instance, const FInspectorSetup& Setup = {})
{
	if (const UClass* Class = T::StaticClass(); IsValid(Class))
	{
		Inspect(Label, *Class, Instance, Setup);
	}
}

}  // namespace ImGuiDeveloperToolkit::PropertyInspector
