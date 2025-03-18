// Copyright 2025 Mikołaj Radwan, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Templates/IsUEnumClass.h"

namespace ImGuiDeveloperToolkit::Widgets
{

bool AutoWidget(const char* Label, const UEnum& Enum, int64& EnumValue);
bool AutoWidget(const char* Label, const UEnum& Enum, const int64& EnumValue);

template <class T UE_REQUIRES(TIsUEnumClass<std::decay_t<T>>::Value)>
bool AutoWidget(const char* Label, T& EnumValue)
{
	const UEnum* Enum = StaticEnum<T>();
	if (!IsValid(Enum))
	{
		return false;
	}

	return AutoWidget(Label, *Enum, EnumValue);
}

}  // namespace ImGuiDeveloperToolkit::Widgets
