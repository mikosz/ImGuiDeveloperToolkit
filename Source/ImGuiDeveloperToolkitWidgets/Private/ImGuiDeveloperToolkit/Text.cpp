// Copyright 2025 Mikołaj Radwan, All Rights Reserved.

#include "ImGuiDeveloperToolkit/Text.h"

#include "imgui.h"

namespace ImGuiDeveloperToolkit::Widgets
{

Zkz::FScopedExecution PushFont(ImFont* const Font)
{
	ZKZ_RETURN_IF(Font == nullptr, {});

	ImGui::PushFont(Font);
	return Zkz::FScopedExecution{&ImGui::PopFont};
}

}  // namespace ImGuiDeveloperToolkit::Widgets
