// Copyright 2025 Mikołaj Radwan, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include <type_traits>

namespace ImGuiDeveloperToolkit::PropertyInspector
{

namespace Private
{

template <typename T, typename = void>
constexpr bool TIsUHTUStruct_v = false;

template <typename T>
constexpr bool TIsUHTUStruct_v<T, std::void_t<decltype(&T::StaticStruct)>> = true;

template <typename T, typename = void>
constexpr bool TIsUHTUClass_v = false;

template <typename T>
constexpr bool TIsUHTUClass_v<T, std::void_t<decltype(&T::StaticClass)>> = true;

template <class From, class To>
using TCopyConstType = std::conditional_t<std::is_const_v<std::remove_reference_t<From>>, std::add_const_t<To>, To>;

static_assert(std::is_same_v<TCopyConstType<const float, int32>, const int32>);
static_assert(std::is_same_v<TCopyConstType<float, int32>, int32>);
static_assert(std::is_same_v<TCopyConstType<const float&, int32>, const int32>);
static_assert(std::is_same_v<TCopyConstType<float&, int32>, int32>);

}  // namespace Private

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
