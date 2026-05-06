// Copyright 2025 Mikołaj Radwan, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Zakazane/RAII.h"

struct ImFont;

namespace ImGuiDeveloperToolkit::Widgets
{

/// Pushes the given font on the stack, returning a scope guard popping it. If Font is null, this is a no-op.
IMGUIDEVELOPERTOOLKITWIDGETS_API [[nodiscard]] Zkz::FScopedExecution PushFont(ImFont* const Font);

// #TODO_dontcommit: consider adding Text functions formatting using variadic templates and implemented for
// all types of unreal strings.

}  // namespace ImGuiDeveloperToolkit::Widgets
