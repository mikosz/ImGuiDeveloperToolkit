// Copyright 2025 Mikołaj Radwan, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "IDetailCustomization.h"

namespace ImGuiDeveloperToolkit::Editor
{

class FShowConfigurationWindowSettingsCustomization final : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
};

}  // namespace ImGuiDeveloperToolkit::Editor
