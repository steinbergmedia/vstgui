// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../lib/vstguifwd.h"

namespace VSTGUI {
class UIAttributes;
class IUIDescription;

//-----------------------------------------------------------------------------
class IViewFactory
{
public:
	virtual ~IViewFactory () noexcept = default;
	
	virtual CView* createView (const UIAttributes& attributes, const IUIDescription* description) const = 0;
	virtual bool applyAttributeValues (CView* view, const UIAttributes& attributes, const IUIDescription* desc) const = 0;
	virtual bool applyCustomViewAttributeValues (CView* customView, IdStringPtr baseViewName, const UIAttributes& attributes, const IUIDescription* desc) const = 0;
};

} // VSTGUI VSTGUI
