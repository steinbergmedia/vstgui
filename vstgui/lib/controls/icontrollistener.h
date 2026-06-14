// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../vstguifwd.h"
#include "../sharedptr.h"
#include "../cbuttonstate.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
class IControlListener : public virtual IReference
{
public:
	virtual ~IControlListener () noexcept = default;

	virtual void valueChanged (CControl& pControl) = 0;
	/** return 1 if you want the control to not handle it, otherwise 0 */
	virtual int32_t controlModifierClicked (CControl& pControl, CButtonState button) = 0;
	virtual void controlBeginEdit (CControl& pControl) = 0;
	virtual void controlEndEdit (CControl& pControl) = 0;
	virtual void controlTagWillChange (CControl& pControl) = 0;
	virtual void controlTagDidChange (CControl& pControl) = 0;
};

//------------------------------------------------------------------------
class ControlListenerAdapter : public IControlListener
{
public:
	void valueChanged (CControl& pControl) override {}
	int32_t controlModifierClicked (CControl& pControl, CButtonState button) override { return 0; }
	void controlBeginEdit (CControl& pControl) override {}
	void controlEndEdit (CControl& pControl) override {}
	void controlTagWillChange (CControl& pControl) override {}
	void controlTagDidChange (CControl& pControl) override {}
};

} // VSTGUI
