// Copyright 2025 Mikołaj Radwan, All Rights Reserved.

#pragma once

namespace ImGuiDeveloperToolkit::Private
{

// #TODO_dontcommit: This really needs to go to a separate utility plugin (with tests)

class FEnumValue
{
public:
	FEnumValue() = default;

	explicit FEnumValue(const UEnum* InEnumClass, const int32 InIndex) : EnumClass{InEnumClass}, Index{InIndex}
	{
		ensure(IsValid(EnumClass.Get()));
	}

	int32 GetIndex() const
	{
		return Index;
	}

	int64 GetValue() const
	{
		return EnumClass->GetValueByIndex(Index);
	}

	FText GetDisplayName() const
	{
		return EnumClass->GetDisplayNameTextByIndex(Index);
	}

	FString GetName() const
	{
		return EnumClass->GetNameStringByIndex(Index);
	}

	FName GetFName() const
	{
		return EnumClass->GetNameByIndex(Index);
	}

	const UEnum* GetEnumClass() const
	{
		return EnumClass.Get();
	}

private:
	TStrongObjectPtr<const UEnum> EnumClass;

	int32 Index = 0;
};

template <class EnumType UE_REQUIRES(TIsUEnumClass<EnumType>::Value)>
class TEnumValue : public FEnumValue
{
public:
	using FEnumValue::FEnumValue;

	TEnumValue(const FEnumValue& Other) : FEnumValue{Other}
	{
	}

	EnumType GetValue() const
	{
		return static_cast<EnumType>(FEnumValue::GetValue());
	}
};

class FEnumValueIterator
{
public:
	FEnumValueIterator() = default;

	explicit FEnumValueIterator(const UEnum* InEnumClass) : CurrentValue{InEnumClass, 0}
	{
	}

	FEnumValueIterator& operator++()
	{
		CurrentValue = FEnumValue{CurrentValue.GetEnumClass(), CurrentValue.GetIndex() + 1};
		return *this;
	}

	const FEnumValue& operator*() const
	{
		return CurrentValue;
	}

	const FEnumValue* operator->() const
	{
		return &CurrentValue;
	}

	explicit operator bool() const
	{
		const UEnum* EnumClass = CurrentValue.GetEnumClass();
		return IsValid(EnumClass)
			   && CurrentValue.GetIndex() < EnumClass->NumEnums() - (EnumClass->ContainsExistingMax() ? 1 : 0);
	}

	friend bool operator==(FEnumValueIterator Lhs, FEnumValueIterator Rhs)
	{
		return !Lhs && !Rhs;
	}

	friend bool operator!=(FEnumValueIterator Lhs, FEnumValueIterator Rhs)
	{
		return !(Lhs == Rhs);
	}

private:
	FEnumValue CurrentValue;
};

template <class EnumType UE_REQUIRES(TIsUEnumClass<EnumType>::Value)>
class TEnumValueIterator
{
public:
	enum class EEnd
	{
		End
	};

	TEnumValueIterator() : CurrentValue{StaticEnum<EnumType>(), 0}
	{
	}

	explicit TEnumValueIterator(EEnd) : CurrentValue{nullptr, 0}
	{
	}

	FEnumValueIterator& operator++()
	{
		CurrentValue = FEnumValue{CurrentValue.GetEnumClass(), CurrentValue.GetIndex() + 1};
		return *this;
	}

	explicit operator bool() const
	{
		const UEnum* EnumClass = CurrentValue.GetEnumClass();
		return IsValid(EnumClass)
			   && CurrentValue.GetIndex() < EnumClass->NumEnums() - (EnumClass->ContainsExistingMax() ? 1 : 0);
	}

	const TEnumValue<EnumType>& operator*() const
	{
		return CurrentValue;
	}

	const TEnumValue<EnumType>& operator->() const
	{
		return &CurrentValue;
	}

	friend bool operator==(TEnumValueIterator Lhs, TEnumValueIterator Rhs)
	{
		return !Lhs && !Rhs;
	}

	friend bool operator!=(TEnumValueIterator Lhs, TEnumValueIterator Rhs)
	{
		return !(Lhs == Rhs);
	}

private:
	TEnumValue<EnumType> CurrentValue;
};

template <class EnumType UE_REQUIRES(TIsUEnumClass<EnumType>::Value)>
class TEnumValueRange
{
	friend TEnumValueIterator<EnumType> begin(TEnumValueRange)
	{
		return {};
	}

	friend TEnumValueIterator<EnumType> end(TEnumValueRange)
	{
		return {};
	}
};

class FEnumValueRange
{
public:
	explicit FEnumValueRange(const UEnum* InEnumClass) : EnumClass{InEnumClass}
	{
	}

private:
	TStrongObjectPtr<const UEnum> EnumClass;

	friend FEnumValueIterator begin(const FEnumValueRange& Range)
	{
		return FEnumValueIterator{Range.EnumClass.Get()};
	}

	friend FEnumValueIterator end(const FEnumValueRange& Range)
	{
		return FEnumValueIterator{};
	}
};

class FMaskEnumValueIterator
{
public:
	FMaskEnumValueIterator() = default;

	explicit FMaskEnumValueIterator(const UEnum* InEnumClass, const int64 InMask) : It{InEnumClass}, Mask{InMask}
	{
		IterateToSelectedValue();
	}

	FMaskEnumValueIterator& operator++()
	{
		++It;
		IterateToSelectedValue();
		return *this;
	}

	FEnumValue operator*() const
	{
		return *It;
	}

	explicit operator bool() const
	{
		return !!It;
	}

	friend bool operator==(const FMaskEnumValueIterator& Lhs, const FMaskEnumValueIterator& Rhs)
	{
		return !Lhs && !Rhs;
	}

	friend bool operator!=(const FMaskEnumValueIterator& Lhs, const FMaskEnumValueIterator& Rhs)
	{
		return !(Lhs == Rhs);
	}

private:
	FEnumValueIterator It;

	int64 Mask = 0;

	void IterateToSelectedValue()
	{
		for (; It; ++It)
		{
			if ((*It).GetValue() & Mask)
			{
				return;
			}
		}
	}
};

template <class EnumType UE_REQUIRES(TIsUEnumClass<EnumType>::Value)>
class TMaskEnumValueIterator : public FMaskEnumValueIterator
{
public:
	TMaskEnumValueIterator() = default;

	explicit TMaskEnumValueIterator(EnumType InMask)
		: FMaskEnumValueIterator{StaticEnum<EnumType>(), static_cast<int64>(InMask)}
	{
	}

	TEnumValue<EnumType> operator*() const
	{
		return TEnumValue<EnumType>{FMaskEnumValueIterator::operator*()};
	}
};

class FMaskEnumValueRange
{
public:
	FMaskEnumValueRange(const UEnum* InEnumClass, int64 InMask) : EnumClass{InEnumClass}, Mask{InMask}
	{
	}

private:
	TStrongObjectPtr<const UEnum> EnumClass;

	int64 Mask = 0;

	friend FMaskEnumValueIterator begin(const FMaskEnumValueRange& Range)
	{
		return FMaskEnumValueIterator{Range.EnumClass.Get(), Range.Mask};
	}

	friend FMaskEnumValueIterator end(const FMaskEnumValueRange& Range)
	{
		return {};
	}
};

template <class EnumType UE_REQUIRES(TIsUEnumClass<EnumType>::Value)>
class TMaskEnumValueRange
{
public:
	explicit TMaskEnumValueRange(EnumType InMask) : Mask{InMask}
	{
	}

private:
	EnumType Mask = 0;

	friend TMaskEnumValueIterator<EnumType> begin(const TMaskEnumValueRange& Range)
	{
		return TMaskEnumValueIterator<EnumType>{Range.Mask};
	}

	friend TMaskEnumValueIterator<EnumType> end(const TMaskEnumValueRange& Range)
	{
		return {};
	}
};

}  // namespace ImGuiDeveloperToolkit::Private
