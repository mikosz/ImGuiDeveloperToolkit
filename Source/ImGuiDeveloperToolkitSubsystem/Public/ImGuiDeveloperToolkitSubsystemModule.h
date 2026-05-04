// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

namespace ImGuiDeveloperToolkit
{

class FImGuiDeveloperToolkitSubsystemModule final : public IModuleInterface
{
public:
	// ~ IModuleInterface
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	// ~ IModuleInterface
};

}  // namespace ImGuiDeveloperToolkit
