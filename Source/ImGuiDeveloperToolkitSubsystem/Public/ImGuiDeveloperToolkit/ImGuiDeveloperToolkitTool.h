// Copyright ZAKAZANE Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "UObject/Object.h"

#include "ImGuiDeveloperToolkitTool.generated.h"

UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = true))
enum class EImGuiDeveloperToolkitToolContext : uint8
{
	Unknown = 0b00 UMETA(Hidden),
	Editor = 0b01,
	Game = 0b10,
};

ENUM_CLASS_FLAGS(EImGuiDeveloperToolkitToolContext);

/// Superclass for Developer Toolkit tools. All non-abstract subclasses will automatically be added to the Developer
/// Toolkit, unless ShouldCreateTool returns false.
UCLASS()
class IMGUIDEVELOPERTOOLKITSUBSYSTEM_API UImGuiDeveloperToolkitTool : public UObject
{
	GENERATED_BODY()
public:
	/// Whether the tool should be created in the current context. Called on the class default object.
	virtual bool ShouldCreateTool() const;

	/// @return Name of the tool as shown in the toolkit window.
	virtual FAnsiString GetToolName() const
		PURE_VIRTUAL(UImGuiDeveloperToolkitTool::GetToolName, return "-- INVALID NAME --";);

	/// @return Whether the tool is available in editor, game or both.
	virtual UPARAM(meta = (Bitmask, BitmaskEnum = EImGuiDeveloperToolkitToolContext)) EImGuiDeveloperToolkitToolContext
		GetContext() const
		PURE_VIRTUAL(UImGuiDeveloperToolkitTool::GetContext, return EImGuiDeveloperToolkitToolContext::Unknown;);

	/// Called every frame by the DeveloperToolkitSubsystem if the tool is within the current context (i.e. EditorOnly
	/// tools show only in editor when not simulating the game, GameOnly show only when simulating the game)
	/// - even when not shown.
	///
	/// @param DeltaTime time since last tick in seconds
	/// @param bInOutShown whether the window is currently shown. Can be internally changed to true by the tool.
	///		Note that this parameter has no effect in either the subsystem or the tool itself. It's just
	///		a parameter stored here for ease of access.
	/// @param Context context in which the toolkit is being run
	/// @param World the level editor world or the game world we're operating in
	/// @return whether the tool should be shown
	virtual void Tick(float DeltaTime, bool& bInOutShown, EImGuiDeveloperToolkitToolContext Context, UWorld* World)
		PURE_VIRTUAL(UImGuiDeveloperToolkit::Tick);
};
