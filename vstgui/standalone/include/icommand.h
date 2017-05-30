// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../../lib/cstring.h"
#include "interface.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {

//------------------------------------------------------------------------
/** %Command definition
 *
 *	Commands are automatically dispatched to Application::IDelegate and IWindowController instances
 *	if they implement the ICommandHandler interface.
 *	Commands are registered via IApplication::registerCommand.
 *
 *
 *	@ingroup standalone
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
inline bool operator!= (const Command& c1, const Command& c2)
{
	return c1.group != c2.group || c1.name != c2.name;
}

//------------------------------------------------------------------------
/** Handler for commands
 *
 *	@ingroup standalone
 */
class ICommandHandler : public Interface
{
public:
	/** Check if command can be handled. */
	virtual bool canHandleCommand (const Command& command) = 0;
	/** Handle command. */
	virtual bool handleCommand (const Command& command) = 0;
};

//------------------------------------------------------------------------
/** predefined command groups
 *
 *	@ingroup standalone
 */
namespace CommandGroup {

static constexpr IdStringPtr Application = "Application";
static constexpr IdStringPtr File = "File";
static constexpr IdStringPtr Edit = "Edit";
static constexpr IdStringPtr Window = "Window";

//------------------------------------------------------------------------
} // CommandGroup

//------------------------------------------------------------------------
namespace CommandName {

static constexpr IdStringPtr About = "About";
static constexpr IdStringPtr Preferences = "Preferences...";
static constexpr IdStringPtr Quit = "Quit";
static constexpr IdStringPtr Help = "Help";
static constexpr IdStringPtr New = "New";
static constexpr IdStringPtr Open = "Open...";
static constexpr IdStringPtr Save = "Save";
static constexpr IdStringPtr SaveAs = "Save As...";
static constexpr IdStringPtr CloseWindow = "Close Window";
static constexpr IdStringPtr Undo = "Undo";
static constexpr IdStringPtr Redo = "Redo";
static constexpr IdStringPtr Cut = "Cut";
static constexpr IdStringPtr Copy = "Copy";
static constexpr IdStringPtr Paste = "Paste";
static constexpr IdStringPtr Delete = "Delete";
static constexpr IdStringPtr SelectAll = "Select All";

static constexpr IdStringPtr MenuSeparator = "~";

} // CommandName

//------------------------------------------------------------------------
/** predefined commands
 *
 *	@ingroup standalone
 */
namespace Commands {

static const Command About {CommandGroup::Application, CommandName::About};
static const Command Preferences {CommandGroup::Application, CommandName::Preferences};
static const Command Quit {CommandGroup::Application, CommandName::Quit};
static const Command Help {CommandGroup::Application, CommandName::Help};

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
static const Command Delete {CommandGroup::Edit, CommandName::Delete};
static const Command SelectAll {CommandGroup::Edit, CommandName::SelectAll};

//------------------------------------------------------------------------
} // Commands

//------------------------------------------------------------------------
} // Standalone
} // VSTGUI
