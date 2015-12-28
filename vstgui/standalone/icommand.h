#pragma once

#include "interface.h"
#include "../lib/cstring.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {

//------------------------------------------------------------------------
/** Commands
 *
 *	Commands are automatically dispatched to Application::IDelegate and IWindowController instances
 *	if they implement the ICommandHandler interface.
 *	Commands are registered via IApplication::registerCommand.
 *
 */
struct Command
{
	UTF8String group;
	UTF8String name;
};

//------------------------------------------------------------------------
inline bool operator== (const Command& c1, const Command& c2)
{
	return c1.group == c2.group && c1.name == c2.name;
}

//------------------------------------------------------------------------
/** Handler for commands */
class ICommandHandler : public Interface
{
public:
	virtual bool canHandleCommand (const Command& command) = 0;
	virtual bool handleCommand (const Command& command) = 0;
};

//------------------------------------------------------------------------
/** predefined command groups */
namespace CommandGroup {

static constexpr IdStringPtr Application = "Application";
static constexpr IdStringPtr File = "File";
static constexpr IdStringPtr Edit = "Edit";

//------------------------------------------------------------------------
} // CommandGroup

//------------------------------------------------------------------------
namespace CommandName {

static constexpr IdStringPtr About = "About";
static constexpr IdStringPtr Preferences = "Preferences";
static constexpr IdStringPtr Quit = "Quit";
static constexpr IdStringPtr New = "New";
static constexpr IdStringPtr Open = "Open";
static constexpr IdStringPtr Save = "Save";
static constexpr IdStringPtr SaveAs = "Save As";
static constexpr IdStringPtr CloseWindow = "Close Window";
static constexpr IdStringPtr Undo = "Undo";
static constexpr IdStringPtr Redo = "Redo";
static constexpr IdStringPtr Cut = "Cut";
static constexpr IdStringPtr Copy = "Copy";
static constexpr IdStringPtr Paste = "Paste";
static constexpr IdStringPtr SelectAll = "Select All";

static constexpr IdStringPtr MenuSeparator = "~";
	
} // CommandName

//------------------------------------------------------------------------
/** predefined commands */
namespace Commands {

static const Command About {CommandGroup::Application, CommandName::About};
static const Command Preferences {CommandGroup::Application, CommandName::Preferences};
static const Command Quit {CommandGroup::Application, CommandName::Quit};

static const Command NewDocument {CommandGroup::File, CommandName::New};
static const Command OpenDocument {CommandGroup::File, CommandName::Open};
static const Command SaveDocument {CommandGroup::File, CommandName::Save};
static const Command SaveDocumentAs {CommandGroup::File, CommandName::SaveAs};
static const Command CloseWindow {CommandGroup::File, CommandName::CloseWindow};

static const Command Undo {CommandGroup::Edit, CommandName::Undo};
static const Command Redo {CommandGroup::Edit, CommandName::Redo};
static const Command Cut {CommandGroup::Edit, CommandName::Cut};
static const Command Copy {CommandGroup::Edit, CommandName::Copy};
static const Command Paste {CommandGroup::Edit, CommandName::Paste};
static const Command SelectAll {CommandGroup::Edit, CommandName::SelectAll};

//------------------------------------------------------------------------
} // Commands

//------------------------------------------------------------------------
} // Standalone
} // VSTGUI
