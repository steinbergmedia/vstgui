// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../vstguifwd.h"

namespace VSTGUI {

//------------------------------------------------------------------------
/** Command menu item target
 *	@ingroup new_in_4_7
 */
class ICommandMenuItemTarget : public virtual IReference
{
public:
	/** called before the item is shown to validate its state */
	virtual bool validateCommandMenuItem (CCommandMenuItem* item) = 0;
	/** called when the item was selected */
	virtual bool onCommandMenuItemSelected (CCommandMenuItem* item) = 0;
};

//------------------------------------------------------------------------
class CommandMenuItemTargetAdapter : public ICommandMenuItemTarget
{
public:
	bool validateCommandMenuItem (CCommandMenuItem* item) override { return false; }
	bool onCommandMenuItemSelected (CCommandMenuItem* item) override { return false; }
};

//------------------------------------------------------------------------
} // VSTGUI
