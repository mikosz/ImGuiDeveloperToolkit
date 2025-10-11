// Copyright 2025 Mikołaj Radwan, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/// Casts a FString to a utf8 const char* - this creates a pointer to a temporary object, so use only as argument
/// to function.
#define IGDT_STRING_TO_CSTR(String) reinterpret_cast<const char*>(StringCast<UTF8CHAR>(*(String)).Get())

/// Casts a FText to a utf8 const char* - this creates a pointer to a temporary object, so use only as argument
/// to function.
#define IGDT_TEXT_TO_CSTR(Text) reinterpret_cast<const char*>(StringCast<UTF8CHAR>(*(Text).ToString()).Get())