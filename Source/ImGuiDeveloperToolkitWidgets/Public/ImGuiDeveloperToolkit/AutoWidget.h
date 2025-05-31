// Copyright 2025 Mikołaj Radwan, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Templates/IsUEnumClass.h"
#include "imgui.h"

namespace ImGuiDeveloperToolkit::Widgets
{

namespace Private
{

template <class T>
struct TImGuiScalarInfo
{
};

template <>
struct TImGuiScalarInfo<int8>
{
	static constexpr ImGuiDataType_ DataType = ImGuiDataType_S8;
	static const char* const Format;
};

template <>
struct TImGuiScalarInfo<uint8>
{
	static constexpr ImGuiDataType_ DataType = ImGuiDataType_U8;
	static const char* const Format;
};

template <>
struct TImGuiScalarInfo<int16>
{
	static constexpr ImGuiDataType_ DataType = ImGuiDataType_S16;
	static const char* const Format;
};

template <>
struct TImGuiScalarInfo<uint16>
{
	static constexpr ImGuiDataType_ DataType = ImGuiDataType_U16;
	static const char* const Format;
};

template <>
struct TImGuiScalarInfo<int32>
{
	static constexpr ImGuiDataType_ DataType = ImGuiDataType_S32;
	static const char* const Format;
};

template <>
struct TImGuiScalarInfo<uint32>
{
	static constexpr ImGuiDataType_ DataType = ImGuiDataType_U32;
	static const char* const Format;
};

template <>
struct TImGuiScalarInfo<int64>
{
	static constexpr ImGuiDataType_ DataType = ImGuiDataType_S64;
	static const char* const Format;
};

template <>
struct TImGuiScalarInfo<uint64>
{
	static constexpr ImGuiDataType_ DataType = ImGuiDataType_U64;
	static const char* const Format;
};

template <>
struct TImGuiScalarInfo<float>
{
	static constexpr ImGuiDataType_ DataType = ImGuiDataType_Float;
	static const char* const Format;
};

template <>
struct TImGuiScalarInfo<double>
{
	static constexpr ImGuiDataType_ DataType = ImGuiDataType_Double;
	static const char* const Format;
};

template <class T>
constexpr ImGuiDataType_ TImGuiScalarDataType_V = TImGuiScalarInfo<T>::DataType;

}  // namespace Private

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

bool AutoWidget(const char* Label, bool& BoolValue);
bool AutoWidget(const char* Label, const bool& BoolValue);

bool AutoWidget(const char* Label, FUtf8String& Utf8StringValue);
bool AutoWidget(const char* Label, const FUtf8String& Utf8StringValue);
bool AutoWidget(const char* Label, FUtf8StringView Utf8StringValue);

bool AutoWidget(const char* Label, FString& StringValue);
bool AutoWidget(const char* Label, const FString& StringValue);

template <class CharType>
bool AutoWidget(const char* Label, TStringView<CharType> StringValue)
{
	FUtf8String Utf8String{StringValue};
	return AutoWidget(Label, Utf8String);
}

template <class NumericType UE_REQUIRES(std::is_arithmetic_v<NumericType>)>
bool AutoWidget(const char* Label, NumericType& NumericValue, NumericType Step, NumericType StepFast)
{
	using namespace Private;

	return ImGui::InputScalar(
		"",
		TImGuiScalarDataType_V<NumericType>,
		&NumericValue,
		&Step,
		&StepFast,
		TImGuiScalarInfo<NumericType>::Format);
}

template <class NumericType UE_REQUIRES(std::is_arithmetic_v<NumericType>)>
bool AutoWidget(const char* Label, const NumericType& NumericValue, NumericType Step, NumericType StepFast)
{
	ImGui::BeginDisabled();
	NumericType Value = NumericValue;
	const bool bResult = AutoWidget(Label, Value, NumericValue);
	ImGui::EndDisabled();

	return bResult;
}

}  // namespace ImGuiDeveloperToolkit::Widgets
