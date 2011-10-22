//
//  icontroller.h
//  vstgui
//
//  Created by Arne Scheffler on 8/7/11.
//  Copyright 2011 Steinberg Media Technologies AG. All rights reserved.
//

#ifndef __icontroller__
#define __icontroller__

#include "../lib/controls/ccontrol.h"

namespace VSTGUI {

class UIAttributes;
class IUIDescription;

//-----------------------------------------------------------------------------
/// @brief extension to CControlListener used by UIDescription
/// @ingroup new_in_4_0
//-----------------------------------------------------------------------------
class IController : public CControlListener
{
public:
	virtual int32_t getTagForName (UTF8StringPtr name, int32_t registeredTag) const { return registeredTag; };
	virtual CControlListener* getControlListener (UTF8StringPtr controlTagName) { return this; }
	virtual CView* createView (const UIAttributes& attributes, IUIDescription* description) { return 0; }
	virtual CView* verifyView (CView* view, const UIAttributes& attributes, IUIDescription* description) { return view; }
	virtual IController* createSubController (UTF8StringPtr name, IUIDescription* description) { return 0; }
};

} // namespace

#endif // __icontroller__
