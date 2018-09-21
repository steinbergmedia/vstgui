// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../vstguifwd.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
/** Option menu listener
 *	@ingroup new_in_4_7
 */
class IOptionMenuListener
{
public:
	/** called before the menu pops up */
	virtual void onOptionMenuPrePopup (COptionMenu* menu) = 0;
	/** called after the menu pops up */
	virtual void onOptionMenuPostPopup (COptionMenu* menu) = 0;
};

//-----------------------------------------------------------------------------
class OptionMenuListenerAdapter : public IOptionMenuListener
{
public:
	void onOptionMenuPrePopup (COptionMenu* menu) override {}
	void onOptionMenuPostPopup (COptionMenu* menu) override {}
};

//------------------------------------------------------------------------
} // VSTGUI
