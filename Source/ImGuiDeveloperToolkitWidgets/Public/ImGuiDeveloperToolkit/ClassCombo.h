// Copyright 2025 Mikołaj Radwan, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Zakazane/Mask.h"
#include "Zakazane/ReturnIfMacros.h"

namespace ImGuiDeveloperToolkit::Widgets
{

enum class EClassComboFlags
{
	IncludeAbstract = 1 << 0,
	IncludeDeprecated = 1 << 1,
	DiscardNative = 1 << 2,
	DiscardBlueprint = 1 << 3,
};

using FClassComboMask = Zkz::TMask<EClassComboFlags>;
constexpr FClassComboMask DefaultClassComboMask = FClassComboMask{};

IMGUIDEVELOPERTOOLKITWIDGETS_API bool ClassCombo(
	const char* Label, UClass& ParentClass, UClass*& SelectedClass, FClassComboMask ClassFlags = {});
IMGUIDEVELOPERTOOLKITWIDGETS_API bool ClassCombo(
	const char* Label, UClass& ParentClass, UClass* const& SelectedClass, FClassComboMask ClassFlags = {});

template <class ParentClassType>
bool ClassCombo(const char* Label, UClass*& SelectedClass, const FClassComboMask ClassFlags = {})
{
	UClass* const ParentClass = ParentClassType::StaticClass();
	ZKZ_RETURN_IF_INVALID(ParentClass, false);

	return ClassCombo(Label, *ParentClass, SelectedClass, ClassFlags);
}

template <class ParentClassType>
bool ClassCombo(const char* Label, UClass* const& SelectedClass, const FClassComboMask ClassFlags = {})
{
	UClass* const ParentClass = ParentClassType::StaticClass();
	ZKZ_RETURN_IF_INVALID(ParentClass, false);

	return ClassCombo(Label, *ParentClass, SelectedClass, ClassFlags);
}

template <class ParentClassType>
bool ClassCombo(const char* Label, TSubclassOf<ParentClassType>& SelectedClass, const FClassComboMask ClassFlags = {})
{
	UClass* const ParentClass = ParentClassType::StaticClass();
	ZKZ_RETURN_IF_INVALID(ParentClass, false);

	UClass* SelectedClassCopy = SelectedClass.Get();

	const bool bSelectionChanged = ClassCombo(Label, *ParentClass, SelectedClassCopy, ClassFlags);

	if (bSelectionChanged)
	{
		SelectedClass = SelectedClassCopy;
	}

	return bSelectionChanged;
}

template <class ParentClassType>
bool ClassCombo(
	const char* Label, const TSubclassOf<ParentClassType>& SelectedClass, const FClassComboMask ClassFlags = {})
{
	UClass* const ParentClass = ParentClassType::StaticClass();
	ZKZ_RETURN_IF_INVALID(ParentClass, false);

	return ClassCombo(Label, *ParentClass, SelectedClass, ClassFlags);
}

}  // namespace ImGuiDeveloperToolkit::Widgets

template <>
struct Zkz::TMaxFlag<ImGuiDeveloperToolkit::Widgets::EClassComboFlags>
{
	static constexpr auto Value =
		static_cast<uint8>(ImGuiDeveloperToolkit::Widgets::EClassComboFlags::DiscardBlueprint);
};
