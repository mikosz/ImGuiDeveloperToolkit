// Copyright ZAKAZANE Studio. All Rights Reserved.

#include "ImGuiDeveloperToolkit/ImGuiDeveloperToolkitSubsystem.h"

#include "ImGuiDeveloperToolkit/ImGuiDeveloperToolkitTool.h"
#include "ImGuiDeveloperToolkit/ImGuiDeveloperToolkitWindow.h"

#include <imgui.h>

namespace ImGuiDeveloperToolkitSubsystemPrivate
{

TPair<EImGuiDeveloperToolkitToolContext, UWorld*> GetContext()
{
#if WITH_EDITOR
	if (GIsEditor && IsValid(GEditor) && !IsValid(GEditor->PlayWorld))
	{
		return {EImGuiDeveloperToolkitToolContext::Editor, GEditor->GetEditorWorldContext().World()};
	}

	return {EImGuiDeveloperToolkitToolContext::Game, GEditor->PlayWorld};
#else
	// #TODO_dontcommit probably see how many GEngine->WorldContexts and if one return it?
	return {EImGuiDeveloperToolkitToolContext::Game, nullptr};
#endif
}

bool IsWithinCurrentContext(const UImGuiDeveloperToolkitTool& Tool)
{
	return EnumHasAnyFlags(Tool.GetContext(), GetContext().Get<0>());
}

}  // namespace ImGuiDeveloperToolkitSubsystemPrivate

void UImGuiDeveloperToolkitSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	Configuration.Initialize();
	PopulateTools();
}

void UImGuiDeveloperToolkitSubsystem::ToggleShow()
{
	bShow = !bShow;
}

bool UImGuiDeveloperToolkitSubsystem::IsShow() const
{
	return bShow;
}

void UImGuiDeveloperToolkitSubsystem::Tick(const float DeltaTime)
{
	using namespace ImGuiDeveloperToolkitSubsystemPrivate;

	ImFont* const Font = Configuration.GetFont();
	if (Font != nullptr)
	{
		ImGui::PushFont(Font);
	}

	if (const ImGui::FScopedContext ScopedContext; ScopedContext)
	{
		TickMainWindow(DeltaTime);
		TickConfigurationWindow(DeltaTime);
		TickTools(DeltaTime);
	}

	if (Font != nullptr)
	{
		ImGui::PopFont();
	}
}

TStatId UImGuiDeveloperToolkitSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UImGuiImGuiSubsystem, STATGROUP_Tickables);
}

UWorld* UImGuiDeveloperToolkitSubsystem::GetTickableGameObjectWorld() const
{
	return GetWorld();
}

bool UImGuiDeveloperToolkitSubsystem::IsTickableInEditor() const
{
	return true;
}

bool UImGuiDeveloperToolkitSubsystem::IsTickableWhenPaused() const
{
	return true;
}

void UImGuiDeveloperToolkitSubsystem::PopulateTools()
{
	if (!ensure(IsInGameThread()))
	{
		return;
	}

	const UClass* ToolSuperclass = UImGuiDeveloperToolkitTool::StaticClass();
	if (!ensure(IsValid(ToolSuperclass)))
	{
		return;
	}

	TArray<UClass*> ToolClasses;
	GetDerivedClasses(ToolSuperclass, ToolClasses);
	for (UClass* ToolClass : ToolClasses)
	{
		if (!IsValid(ToolClass) || ToolClass->HasAllClassFlags(CLASS_Abstract))
		{
			continue;
		}

		const UImGuiDeveloperToolkitTool* DefaultTool = GetDefault<UImGuiDeveloperToolkitTool>(ToolClass);
		if (!IsValid(DefaultTool) || !DefaultTool->ShouldCreateTool())
		{
			continue;
		}

		UImGuiDeveloperToolkitTool* Tool = NewObject<UImGuiDeveloperToolkitTool>(this, ToolClass);
		if (!IsValid(Tool))
		{
			continue;
		}

		const bool bAlreadyContains = Tools.ContainsByPredicate(
			[Name = Tool->GetToolName()](const UImGuiDeveloperToolkitTool* ContainedTool)
			{
				if (!IsValid(ContainedTool))
				{
					return false;
				}

				return ContainedTool->GetToolName() == Name;
			});

		if (bAlreadyContains)
		{
			continue;
		}

		Tools.Emplace(MoveTemp(Tool));
	}
}

void UImGuiDeveloperToolkitSubsystem::TickMainWindow(const float DeltaTime)
{
	using namespace ImGuiDeveloperToolkit;

	if (bShow)
	{
		ON_SCOPE_EXIT
		{
			ImGui::End();
		};

		SetNextWindowPosAndSizeWithinMainViewport(ImVec2{.6f, .1f}, ImVec2{.3f, .3f}, ImGuiCond_FirstUseEver);

		if (ImGui::Begin("Developer Toolkit", &bShow, ImGuiWindowFlags_MenuBar))
		{
			TickMainMenu(DeltaTime);
			TickToolList(DeltaTime);
		}
	}
}

void UImGuiDeveloperToolkitSubsystem::TickConfigurationWindow(const float DeltaTime)
{
	Configuration.Tick(DeltaTime);
}

void UImGuiDeveloperToolkitSubsystem::TickMainMenu(const float DeltaTime)
{
	if (ImGui::BeginMenuBar())
	{
		Configuration.bShown = ImGui::MenuItem("Configuration") || Configuration.bShown;
		ImGui::EndMenuBar();
	}
}

void UImGuiDeveloperToolkitSubsystem::TickToolList(const float DeltaTime)
{
	using namespace ImGuiDeveloperToolkitSubsystemPrivate;

	ImGui::InputText("Find tool...", ToolNameFilter.GetData(), ToolNameFilterSize);
	const FAnsiStringView FindToolTextView{ToolNameFilter.GetData()};

	for (UImGuiDeveloperToolkitTool* Tool : Tools)
	{
		if (!IsValid(Tool) || !IsWithinCurrentContext(*Tool))
		{
			continue;
		}

		const FAnsiString ToolName = Tool->GetToolName();
		if (FindToolTextView.IsEmpty() || ToolName.Contains(FindToolTextView))
		{
			if (ImGui::Button(*ToolName))
			{
				Configuration.SetShown(ToolName, true);
				bShow = false;
			}
		}
	}
}

void UImGuiDeveloperToolkitSubsystem::TickTools(const float DeltaTime)
{
	using namespace ImGuiDeveloperToolkitSubsystemPrivate;

	const auto& [Context, World] = GetContext();

	for (UImGuiDeveloperToolkitTool* Tool : Tools)
	{
		if (!IsValid(Tool) || !EnumHasAnyFlags(Tool->GetContext(), Context))
		{
			continue;
		}

		const FAnsiString ToolName = Tool->GetToolName();

		bool bShown = Configuration.IsShown(ToolName);
		Tool->Tick(DeltaTime, bShown, Context, World);

		Configuration.SetShown(ToolName, bShown);
	}
}
