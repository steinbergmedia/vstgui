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
	/** called when the platform optionmenu returns the result and before the value of the option
	 *	menu is set.
	 *	@ingroup new_in_4_10
	 *
	 *	@param menu the listened menu
	 *	@param selectedMenu the menu containing the selected item
	 *	@param selectedIndex the index of the selected item
	 *	@return return true to prevent further propagating the call to other listeners and to
	 *			prevent setting the value of the option menu
	 */
	virtual bool onOptionMenuSetPopupResult (COptionMenu* menu, COptionMenu* selectedMenu,
											 int32_t selectedIndex) = 0;
};

//-----------------------------------------------------------------------------
class OptionMenuListenerAdapter : public IOptionMenuListener
{
public:
	void onOptionMenuPrePopup (COptionMenu* menu) override {}
	void onOptionMenuPostPopup (COptionMenu* menu) override {}
	bool onOptionMenuSetPopupResult (COptionMenu* menu, COptionMenu* selectedMenu,
									 int32_t selectedIndex) override
	{
		return false;
	}
};

//------------------------------------------------------------------------
} // VSTGUI
