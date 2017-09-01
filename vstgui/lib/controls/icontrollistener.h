// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __icontrollistener__
#define __icontrollistener__

#include "../vstguifwd.h"
#include "../cbuttonstate.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
class IControlListener
{
public:
	virtual ~IControlListener () noexcept = default;
	virtual void valueChanged (CControl* pControl) = 0;
	virtual int32_t controlModifierClicked (CControl* pControl, CButtonState button) { return 0; }	///< return 1 if you want the control to not handle it, otherwise 0
	virtual void controlBeginEdit (CControl* pControl) {}
	virtual void controlEndEdit (CControl* pControl) {}
	virtual void controlTagWillChange (CControl* pControl) {}
	virtual void controlTagDidChange (CControl* pControl) {}
};

}

#endif // __icontrollistener__
