#include "ImGuiDeveloperToolkit/DemoImGuiDeveloperToolkitTool.h"

#include "ImGuiDeveloperToolkit/ClassCombo.h"
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

	DemoData->Class.Reset(NewObject<UImGuiDemoDeveloperToolkitTool_ChildClass>(this));
	check(DemoData->Class.IsValid());
}

void UDemoImGuiDeveloperToolkitTool::ShowDemoWindow(bool* Open)
{
	using namespace ImGuiDeveloperToolkit::PropertyInspector;
	using namespace ImGuiDeveloperToolkit::Widgets;

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
		const auto ShowPropertyInspectorSetup =
			[](const char* const Label, FInspectorSetup& Setup, const bool bWithSuperclassSelector)
		{
			if (!ImGui::TreeNodeEx(Label))
			{
				return;
			}

			ON_SCOPE_EXIT
			{
				ImGui::TreePop();
			};

			const auto ShowTypeSetup = [](const char* const Label, FInspectorSetup::FTypeSetup& TypeSetup)
			{
				ImGui::PushID(&TypeSetup);
				if (ImGui::TreeNodeEx(Label))
				{
					ImGui::Checkbox("Recurse into", &TypeSetup.bRecurseInto);
					ImGui::Checkbox("Show categories", &TypeSetup.bShowCategories);
					ImGui::Checkbox("Show hierarchy", &TypeSetup.bShowHierarchy);
					ImGui::TreePop();
				}
				ImGui::PopID();
			};

			if (bWithSuperclassSelector)
			{
				TSubclassOf<UImGuiDemoDeveloperToolkitTool_GrandparentClass> OnlyChildrenOf =
					Cast<UClass>(Setup.OnlyChildrenOf);
				if (ClassCombo(
						"Only children of", OnlyChildrenOf, FClassComboMask{} | EClassComboFlags::IncludeAbstract))
				{
					Setup.OnlyChildrenOf = OnlyChildrenOf;
				}
				
				ImGui::SameLine();
				if (ImGui::Button("Clear"))
				{
					Setup.OnlyChildrenOf = nullptr;
				}
			}

			ImGui::Checkbox("Include deprecated", &Setup.bIncludeDeprecated);
			ShowTypeSetup("Struct setup", Setup.StructSetup);
			ShowTypeSetup("Object setup", Setup.ObjectSetup);

			ImGui::Separator();
		};

		ShowPropertyInspectorSetup("Struct inspector setup", DemoData->PropertyInspectorSetup_Struct, false);
		Inspect("ChildStruct", DemoData->Struct, this, DemoData->PropertyInspectorSetup_Struct);

		ShowPropertyInspectorSetup("Class inspector setup", DemoData->PropertyInspectorSetup_Class, true);
		Inspect("ChildClass", *DemoData->Class, DemoData->PropertyInspectorSetup_Class);
	}
}
