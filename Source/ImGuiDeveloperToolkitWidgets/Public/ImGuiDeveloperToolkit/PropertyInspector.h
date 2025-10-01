// Copyright 2025 Mikołaj Radwan, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Private/TypeTraits.h"

namespace ImGuiDeveloperToolkit::PropertyInspector
{

struct FInspectorSetup
{
	/// When inspecting UStructs or UClasses will only show properties of this type or a child type. E.g. for UObject
	/// if you inspect a UPrimitiveComponent and set this to UActorComponent, will show properties of UPrimitiveComponent,
	/// USceneComponent, and UActorComponent.
	UStruct* OnlyChildrenOf = nullptr;

	/// Whether to also show properties of struct properties.
	bool bRecurseIntoStructs = true;

	/// Whether to also show properties of pointed-to UObjects.
	bool bRecurseIntoObjects = false;

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
	const char* Label, const UStruct& Struct, void* Instance, UObject* OuterObject, const FInspectorSetup& Setup);
IMGUIDEVELOPERTOOLKITWIDGETS_API void Inspect(
	const char* Label,
	const UStruct& Struct,
	const void* Instance,
	const UObject* OuterObject,
	const FInspectorSetup& Setup);
IMGUIDEVELOPERTOOLKITWIDGETS_API void Inspect(
	const char* Label, const UClass& Class, UObject& Instance, const FInspectorSetup& Setup);
IMGUIDEVELOPERTOOLKITWIDGETS_API void Inspect(
	const char* Label, const UClass& Class, const UObject& Instance, const FInspectorSetup& Setup);

template <class T UE_REQUIRES(Private::TIsUHTUStruct_v<T>)>
void Inspect(
	const char* Label, T& Instance, Private::TCopyConstType<T, UObject>* OuterObject, const FInspectorSetup& Setup)
{
	if (const UStruct* Struct = T::StaticStruct(); IsValid(Struct))
	{
		Inspect(Label, *Struct, &Instance, OuterObject, Setup);
	}
}

template <class T UE_REQUIRES(Private::TIsUHTUClass_v<T>)>
void Inspect(const char* Label, T& Instance, const FInspectorSetup& Setup)
{
	if (const UClass* Class = T::StaticClass(); IsValid(Class))
	{
		Inspect(Label, *Class, Instance, Setup);
	}
}

}  // namespace ImGuiDeveloperToolkit::PropertyInspector
