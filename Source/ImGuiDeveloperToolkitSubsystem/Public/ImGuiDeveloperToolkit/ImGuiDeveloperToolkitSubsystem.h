// Copyright ZAKAZANE Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "ImGuiDeveloperToolkitConfiguration.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "ImGuiDeveloperToolkitSubsystem.generated.h"

class UImGuiDeveloperToolkitTool;


/// Stores DeveloperToolkitTools, ticks them and displays a window allowing tool selection and filtering.
UCLASS()
class IMGUIDEVELOPERTOOLKITSUBSYSTEM_API UImGuiDeveloperToolkitSubsystem final : public UEngineSubsystem,
																				 public FTickableGameObject
{
	GENERATED_BODY()
public:
	UPROPERTY(VisibleAnywhere)
	FImGuiDeveloperToolkitConfiguration Configuration;

	// ~ USubsystem
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	// ~ USubsystem

	UFUNCTION(Category = "DeveloperToolkit")
	void ToggleShow();

	UFUNCTION(Category = "DeveloperToolkit")
	bool IsShow() const;

private:
	// ~ FTickableGameObject
	virtual void Tick(float DeltaTime) override;

	virtual TStatId GetStatId() const override;

	virtual UWorld* GetTickableGameObjectWorld() const override;

	virtual bool IsTickableInEditor() const override;

	virtual bool IsTickableWhenPaused() const override;
	// ~ FTickableGameObject

	void PopulateTools();

	void TickMainWindow(float DeltaTime);

	void TickConfigurationWindow(float DeltaTime);

	void TickMainMenu(float DeltaTime);

	void TickToolList(float DeltaTime);

	void TickTools(float DeltaTime);

	UPROPERTY(Transient, VisibleAnywhere)
	bool bShow = false;

	UPROPERTY(Transient, VisibleAnywhere)
	TArray<TObjectPtr<UImGuiDeveloperToolkitTool>> Tools;

	static constexpr size_t ToolNameFilterSize = 128;

	TArray<ANSICHAR, TInlineAllocator<ToolNameFilterSize>> ToolNameFilter;
};
