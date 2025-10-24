// Copyright 2025 Mikołaj Radwan, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Zakazane/ReturnIfMacros.h"

namespace ImGuiDeveloperToolkit::Widgets
{

IMGUIDEVELOPERTOOLKITWIDGETS_API bool ClassCombo(const char* Label, const UClass& ParentClass, UClass*& SelectedClass);
IMGUIDEVELOPERTOOLKITWIDGETS_API bool ClassCombo(
	const char* Label, const UClass& ParentClass, UClass* const& SelectedClass);

template <class ParentClassType>
bool ClassCombo(const char* Label, UClass*& SelectedClass)
{
	UClass* ParentClass = ParentClassType::StaticClass();
	ZKZ_RETURN_IF_INVALID(ParentClass, false);

	return ClassCombo(Label, *ParentClass, SelectedClass);
}

template <class ParentClassType>
bool ClassCombo(const char* Label, UClass* const& SelectedClass)
{
	UClass* ParentClass = ParentClassType::StaticClass();
	ZKZ_RETURN_IF_INVALID(ParentClass, false);

	return ClassCombo(Label, *ParentClass, SelectedClass);
}

template <class ParentClassType>
bool ClassCombo(const char* Label, TSubclassOf<ParentClassType>& SelectedClass)
{
	UClass* ParentClass = ParentClassType::StaticClass();
	ZKZ_RETURN_IF_INVALID(ParentClass, false);

	UClass* SelectedClassCopy = SelectedClass.Get();
	
	const bool bSelectionChanged = ClassCombo(Label, *ParentClass, SelectedClassCopy);
	
	if (bSelectionChanged)
	{
		SelectedClass = SelectedClassCopy;
	}
	
	return bSelectionChanged;
}

template <class ParentClassType>
bool ClassCombo(const char* Label, const TSubclassOf<ParentClassType>& SelectedClass)
{
	UClass* const ParentClass = ParentClassType::StaticClass();
	ZKZ_RETURN_IF_INVALID(ParentClass, false);

	return ClassCombo(Label, *ParentClass, SelectedClass);
}

}  // namespace ImGuiDeveloperToolkit::Widgets
