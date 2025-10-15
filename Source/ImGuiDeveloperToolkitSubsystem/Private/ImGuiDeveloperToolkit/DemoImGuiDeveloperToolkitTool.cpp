#include "DemoImGuiDeveloperToolkitTool.h"

#include "ImGuiDeveloperToolkit/PropertyInspector.h"
#include "Zakazane/ReturnIfMacros.h"
#include "imgui.h"
#include "implot.h"

FAnsiString UDemoImGuiDeveloperToolkitTool::GetToolName() const
{
	return "ImGui Demo";
}

EImGuiDeveloperToolkitToolContext UDemoImGuiDeveloperToolkitTool::GetContext() const
{
	return EImGuiDeveloperToolkitToolContext::Editor | EImGuiDeveloperToolkitToolContext::Game;
}

void UDemoImGuiDeveloperToolkitTool::Tick(
	float DeltaTime, bool& bInOutShow, EImGuiDeveloperToolkitToolContext Context, UWorld* World)
{
	TickDemoSelectionWindow(bInOutShow);
	TickDemoWindows();
}

void UDemoImGuiDeveloperToolkitTool::TickDemoSelectionWindow(bool& bInOutShow)
{
	// #TODO_dontcommit closing tool windows broken after config update
	ZKZ_RETURN_IF(!bInOutShow);

	ON_SCOPE_EXIT
	{
		ImGui::End();
	};

	if (!ImGui::Begin("ImGui Developer Toolkit Demo Selection", &bInOutShow))
	{
		return;
	}

	if (ImGui::Button("Open ImGui Demo"))
	{
		bShowImGuiDemoWindow = true;
	}

	if (ImGui::Button("Open ImPlot Demo"))
	{
		bShowImPlotDemoWindow = true;
	}

	if (ImGui::Button("Open ImGui Developer Tool Demo"))
	{
		bShowImGuiDeveloperToolkitDemoWindow = true;
	}
}

void UDemoImGuiDeveloperToolkitTool::TickDemoWindows()
{
	if (bShowImGuiDemoWindow)
	{
		ImGui::ShowDemoWindow(&bShowImGuiDemoWindow);
	}

	if (bShowImPlotDemoWindow)
	{
		ImPlot::ShowDemoWindow(&bShowImPlotDemoWindow);
	}

	if (bShowImGuiDeveloperToolkitDemoWindow)
	{
		ShowDemoWindow(&bShowImGuiDeveloperToolkitDemoWindow);
	}
}

void UDemoImGuiDeveloperToolkitTool::EnsureHasDemoData()
{
	if (DemoData.IsValid())
	{
		return;
	}

	DemoData = MakeUnique<FStructDemoData>();
	check(DemoData.IsValid());

	DemoData->ChildClass.Reset(NewObject<UImGuiDemoDeveloperToolkitTool_ChildClass>(this));
	check(DemoData->ChildClass.IsValid());
}

void UDemoImGuiDeveloperToolkitTool::ShowDemoWindow(bool* Open)
{
	ON_SCOPE_EXIT
	{
		ImGui::End();
	};

	if (!ImGui::Begin("ImGui Developer Toolkit Demo", Open))
	{
		return;
	}

	EnsureHasDemoData();

	if (ImGui::CollapsingHeader("Property inspector"))
	{
		ImGui::Checkbox("Include deprecated", &DemoData->PropertyInspectorSetup.bIncludeDeprecated);
		ImGui::Checkbox("Recurse into structs", &DemoData->PropertyInspectorSetup.bRecurseIntoStructs);
		ImGui::Checkbox("Recurse into objects", &DemoData->PropertyInspectorSetup.bRecurseIntoObjects);

		ImGuiDeveloperToolkit::PropertyInspector::Inspect(
			"ChildStruct", DemoData->ChildStruct, this, DemoData->PropertyInspectorSetup);
		ImGuiDeveloperToolkit::PropertyInspector::Inspect(
			"ChildClass", *DemoData->ChildClass, DemoData->PropertyInspectorSetup);
	}
}
