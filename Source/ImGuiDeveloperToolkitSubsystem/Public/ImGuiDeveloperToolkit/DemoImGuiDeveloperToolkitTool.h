// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "ImGuiDeveloperToolkit/ImGuiDeveloperToolkitTool.h"
#include "ImGuiDeveloperToolkit/PropertyInspector.h"

#include "DemoImGuiDeveloperToolkitTool.generated.h"

// #TODO_dontcommit: Demo should go to a separate module and be present only if loaded!

/// Grandparent struct tooltip - docstring
USTRUCT(DisplayName = "Grandparent struct")
struct IMGUIDEVELOPERTOOLKITSUBSYSTEM_API FImGuiDemoDeveloperToolkitTool_GrandparentStruct
{
	GENERATED_BODY()

	/// String property tooltip - docstring
	UPROPERTY(Category = "Cat A", EditAnywhere, DisplayName = "Grandparent property (string)")
	FString GrandparentPropertyString = TEXT("Grandparent property");
};

USTRUCT(DisplayName = "Parent struct", meta = (ToolTip = "Parent struct tooltip - meta"))
struct IMGUIDEVELOPERTOOLKITSUBSYSTEM_API FImGuiDemoDeveloperToolkitTool_ParentStruct
	: public FImGuiDemoDeveloperToolkitTool_GrandparentStruct
{
	GENERATED_BODY()

	UPROPERTY(
		Category = "Cat B",
		EditAnywhere,
		DisplayName = "Parent property (bool)",
		meta = (ToolTip = "Bool property tooltip - meta"))
	bool bParentProperty = false;
};

USTRUCT(DisplayName = "Child struct")
struct IMGUIDEVELOPERTOOLKITSUBSYSTEM_API FImGuiDemoDeveloperToolkitTool_ChildStruct
	: public FImGuiDemoDeveloperToolkitTool_ParentStruct
{
	GENERATED_BODY()

	UPROPERTY(Category = "Cat A", EditAnywhere, DisplayName = "Child property (int32)")
	int32 ParentProperty = 42;
};

UCLASS(Blueprintable, DisplayName = "Grandparent class")
class IMGUIDEVELOPERTOOLKITSUBSYSTEM_API UImGuiDemoDeveloperToolkitTool_GrandparentClass : public UObject
{
	GENERATED_BODY()
};

/// Parent class tooltip - docstring
UCLASS(DisplayName = "Parent class")
class IMGUIDEVELOPERTOOLKITSUBSYSTEM_API UImGuiDemoDeveloperToolkitTool_ParentClass
	: public UImGuiDemoDeveloperToolkitTool_GrandparentClass
{
	GENERATED_BODY()

public:
	UPROPERTY(Category = "Cat A", EditAnywhere, DisplayName = "Parent class property (ChildStruct)")
	FImGuiDemoDeveloperToolkitTool_ChildStruct ChildStructPropertyInParent;

	UPROPERTY(Category = "Cat B", EditAnywhere, DisplayName = "Parent class property (Text)")
	FText TextPropertyInParent = FText::FromString("Text property");
};

UCLASS(DisplayName = "Child class", meta = (ToolTip = "Child class tooltip - meta"))
class IMGUIDEVELOPERTOOLKITSUBSYSTEM_API UImGuiDemoDeveloperToolkitTool_ChildClass
	: public UImGuiDemoDeveloperToolkitTool_ParentClass
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
class IMGUIDEVELOPERTOOLKITSUBSYSTEM_API UDemoImGuiDeveloperToolkitTool : public UImGuiDeveloperToolkitTool
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
		ImGuiDeveloperToolkit::PropertyInspector::FInspectorSetup PropertyInspectorSetup_Struct;
		FImGuiDemoDeveloperToolkitTool_ChildStruct Struct;

		ImGuiDeveloperToolkit::PropertyInspector::FInspectorSetup PropertyInspectorSetup_Class;
		TStrongObjectPtr<UImGuiDemoDeveloperToolkitTool_ChildClass> Class;
	};

	TUniquePtr<FStructDemoData> DemoData;

	void TickDemoSelectionWindow(bool& bInOutShow);
	void TickDemoWindows();

	void EnsureHasDemoData();
	void ShowDemoWindow(bool* Open);
};
