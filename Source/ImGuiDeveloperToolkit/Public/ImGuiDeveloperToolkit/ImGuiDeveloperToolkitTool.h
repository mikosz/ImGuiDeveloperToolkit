// Copyright ZAKAZANE Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "UObject/Object.h"

#include "ImGuiDeveloperToolkitTool.generated.h"

UENUM()
enum class EImGuiDeveloperToolkitToolContext
{
	Unknown,
	EditorOnly,
	GameOnly,
	EditorAndGame,
};

/**
 * Superclass for Developer Toolkit tools. All non-abstract subclasses will automatically be added to the Developer
 * Toolkit, unless ShouldCreateTool returns false.
 */
UCLASS()
class IMGUIDEVELOPERTOOLKIT_API UImGuiDeveloperToolkitTool : public UObject
{
	GENERATED_BODY()
public:
	/**
	 * Whether the tool should be created in the current context. Called on the class default object. 
	 */
	virtual bool ShouldCreateTool() const;

	/**
	 * @return Name of the tool as shown in the toolkit window. 
	 */
	virtual FAnsiString GetToolName() const
		PURE_VIRTUAL(UImGuiDeveloperToolkitTool::GetToolName, return "-- INVALID NAME --";);

	/**
	 * @return Whether the tool is available in editor, game or both. 
	 */
	virtual EImGuiDeveloperToolkitToolContext GetContext() const
		PURE_VIRTUAL(UImGuiDeveloperToolkitTool::GetContext, return EImGuiDeveloperToolkitToolContext::Unknown;);

	/**
	 * Marks the tool as shown. Note that this does not display any windows, the information is just passed
	 * down to the implementation class to be handled there.
	 */
	void Show();

	void Tick(float DeltaTime);

protected:
	/**
	 * Called every frame by the DeveloperToolkitSubsystem if the tool is within the current context (i.e. EditorOnly
	 * tools show only in editor when not simulating the game, GameOnly show only when simulating the game)
	 * - even when not shown.
	 * @param bInOutShow whether the window is currently shown. Can be internally changed to true by the tool.
	 *		Note that this parameter has no effect in either the subsystem or the tool itself. It's just
	 *		a parameter stored here for ease of access.
	 */
	virtual void DoTick(float DeltaTime, bool& bInOutShow) PURE_VIRTUAL(UImGuiDeveloperToolkit::DoTick);

private:
	bool bShow = false;
};
