// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "ImGuiDeveloperToolkit/ImGuiDeveloperToolkitTool.h"
#include "ImGuiDeveloperToolkit/PropertyInspector.h"

#include "DemoImGuiDeveloperToolkitTool.generated.h"

/// Grandparent struct tooltip - docstring
USTRUCT(DisplayName = "Grandparent struct")
struct FImGuiDemoDeveloperToolkitTool_GrandparentStruct
{
	GENERATED_BODY()

	/// String property tooltip - docstring
	UPROPERTY(Category = "Cat A", EditAnywhere, DisplayName = "Grandparent property (string)")
	FString GrandparentPropertyString = TEXT("Grandparent property");
};

USTRUCT(DisplayName = "Parent struct", meta = (ToolTip = "Parent struct tooltip - meta"))
struct FImGuiDemoDeveloperToolkitTool_ParentStruct : public FImGuiDemoDeveloperToolkitTool_GrandparentStruct
{
	GENERATED_BODY()

	UPROPERTY(Category = "Cat B", EditAnywhere, DisplayName = "Parent property (bool)", meta = (ToolTip = "Bool property tooltip - meta"))
	bool bParentProperty = false;
};

USTRUCT(DisplayName = "Child struct")
struct FImGuiDemoDeveloperToolkitTool_ChildStruct : public FImGuiDemoDeveloperToolkitTool_ParentStruct
{
	GENERATED_BODY()

	UPROPERTY(Category = "Cat A", EditAnywhere, DisplayName = "Child property (int32)")
	int32 ParentProperty = 42;
};

/// Parent class tooltip - docstring
UCLASS(DisplayName = "Parent class")
class UImGuiDemoDeveloperToolkitTool_ParentClass : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(Category = "Cat A", EditAnywhere, DisplayName = "Parent class property (ChildStruct)")
	FImGuiDemoDeveloperToolkitTool_ChildStruct ChildStructPropertyInParent;

	UPROPERTY(Category = "Cat B", EditAnywhere, DisplayName = "Parent class property (Text)")
	FText TextPropertyInParent = FText::FromString("Text property");
};

UCLASS(DisplayName = "Child class", meta = (ToolTip = "Child class tooltip - meta"))
class UImGuiDemoDeveloperToolkitTool_ChildClass : public UImGuiDemoDeveloperToolkitTool_ParentClass
{
	GENERATED_BODY()

public:
	UPROPERTY(Category = "Cat A", EditAnywhere, DisplayName = "Child class property (ChildStruct)")
	FImGuiDemoDeveloperToolkitTool_ChildStruct ChildStructPropertyInChild;

	UPROPERTY(Category = "Cat B", EditAnywhere, DisplayName = "Child class property (int32)")
	int32 Int32PropertyInChild = 12;

	UPROPERTY(DisplayName = "Child class deprecated property (int32)", meta = (DeprecatedProperty))
	int32 DeprecatedPropertyInChild_DEPRECATED = 666;
};

UCLASS()
class UDemoImGuiDeveloperToolkitTool : public UImGuiDeveloperToolkitTool
{
	GENERATED_BODY()
public:
	virtual FAnsiString GetToolName() const override;
	virtual EImGuiDeveloperToolkitToolContext GetContext() const override;
	virtual void Tick(
		float DeltaTime, bool& bInOutShow, EImGuiDeveloperToolkitToolContext Context, UWorld* World) override;

private:
	bool bShowImGuiDemoWindow = false;
	bool bShowImPlotDemoWindow = false;
	bool bShowImGuiDeveloperToolkitDemoWindow = false;

	struct FStructDemoData
	{
		ImGuiDeveloperToolkit::PropertyInspector::FInspectorSetup PropertyInspectorSetup;

		FImGuiDemoDeveloperToolkitTool_ChildStruct ChildStruct;
		TStrongObjectPtr<UImGuiDemoDeveloperToolkitTool_ChildClass> ChildClass;
	};

	TUniquePtr<FStructDemoData> DemoData;

	void TickDemoSelectionWindow(bool& bInOutShow);
	void TickDemoWindows();

	void EnsureHasDemoData();
	void ShowDemoWindow(bool* Open);
};
