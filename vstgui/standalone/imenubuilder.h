#pragma once

#include "fwd.h"
#include "interface.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {

//------------------------------------------------------------------------
/** Menu builder interface
 *
 *	Application delegates and window controllers can implement this interface to customize the
 *	visibility and order of commands shown in the menu of the application or window.
 *	The context parameter of the methods is either an IApplication or IWindow.
 *
 *	@ingroup standalone
 */
class IMenuBuilder : public Interface
{
public:
	using SortFunction = std::function<bool (const UTF8String& lhs, const UTF8String& rhs)>;

	/** should the command group be visible in the menu
	 *
	 *	@param context either an IApplication or IWindow instance
	 *	@param group group name
	 *	@return true for visible or false for invisible
	 */
	virtual bool showCommandGroupInMenu (const Interface& context,
	                                     const UTF8String& group) const = 0;
	/** should the command be visible in the menu
	 *
	 *	@param context either an IApplication or IWindow instance
	 *	@param cmd command
	 *	@return true for visible or false for invisible
	 */
	virtual bool showCommandInMenu (const Interface& context, const Command& cmd) const = 0;
	/** return command group sort function
	 *
	 *	@param context either an IApplication or IWindow instance
	 *	@param group group name
	 *	@return if you want to sort the menu return a SortFunction otherwise return nullptr
	 */
	virtual SortFunction getCommandGroupSortFunction (const Interface& context,
	                                                  const UTF8String& group) const = 0;
	/** should a menu separator prepend a command
	 *
	 *	@param context either an IApplication or IWindow instance
	 *	@param cmd command
	 *	@return true if a menu separator should be prepended before the command
	 */
	virtual bool prependMenuSeparator (const Interface& context, const Command& cmd) const = 0;
};

//------------------------------------------------------------------------
} // Standalone
} // VSTGUI
