// Copyright 2025 Mikołaj Radwan, All Rights Reserved.

#pragma once

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

}  // namespace Private

IMGUIDEVELOPERTOOLKITWIDGETS_API void Inspect(const char* Label, const UStruct& Struct, void* Instance);
IMGUIDEVELOPERTOOLKITWIDGETS_API void Inspect(const char* Label, const UStruct& Struct, const void* Instance);

template <class T UE_REQUIRES(Private::TIsUHTUStruct_v<T>)>
void Inspect(const char* Label, T* Instance)
{
	if (const UStruct* Struct = T::StaticStruct(); IsValid(Struct))
	{
		Inspect(Label, *Struct, Instance);
	}
}

template <class T UE_REQUIRES(Private::TIsUHTUClass_v<T>)>
void Inspect(const char* Label, T* Instance)
{
	if (const UClass* Class = T::StaticClass; IsValid(Class))
	{
		Inspect(Label, *Class, Instance);
	}
}

}  // namespace ImGuiDeveloperToolkit::PropertyInspector
