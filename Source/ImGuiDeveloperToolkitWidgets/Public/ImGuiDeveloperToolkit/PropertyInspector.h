// Copyright 2025 Mikołaj Radwan, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Private/TypeTraits.h"

namespace ImGuiDeveloperToolkit::PropertyInspector
{

IMGUIDEVELOPERTOOLKITWIDGETS_API void Inspect(
	const char* Label, const UStruct& Struct, void* Instance, UObject* OuterObject);
IMGUIDEVELOPERTOOLKITWIDGETS_API void Inspect(
	const char* Label, const UStruct& Struct, const void* Instance, const UObject* OuterObject);
IMGUIDEVELOPERTOOLKITWIDGETS_API void Inspect(const char* Label, const UClass& Class, UObject& Instance);
IMGUIDEVELOPERTOOLKITWIDGETS_API void Inspect(const char* Label, const UClass& Class, const UObject& Instance);

template <class T UE_REQUIRES(Private::TIsUHTUStruct_v<T>)>
void Inspect(const char* Label, T& Instance, Private::TCopyConstType<T, UObject>* OuterObject)
{
	if (const UStruct* Struct = T::StaticStruct(); IsValid(Struct))
	{
		Inspect(Label, *Struct, &Instance, OuterObject);
	}
}

template <class T UE_REQUIRES(Private::TIsUHTUClass_v<T>)>
void Inspect(const char* Label, T& Instance)
{
	if (const UClass* Class = T::StaticClass(); IsValid(Class))
	{
		Inspect(Label, *Class, Instance);
	}
}

}  // namespace ImGuiDeveloperToolkit::PropertyInspector
