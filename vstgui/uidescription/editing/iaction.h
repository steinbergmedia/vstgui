// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __iaction__
#define __iaction__

#include "../../lib/vstguifwd.h"

#if VSTGUI_LIVE_EDITING
#include "../../lib/cfont.h"

namespace VSTGUI {
class UIAttributes;

//----------------------------------------------------------------------------------------------------
class IAction
{
public:
	virtual ~IAction () {}
	
	virtual UTF8StringPtr getName () = 0;
	virtual void perform () = 0;
	virtual void undo () = 0;
};

//----------------------------------------------------------------------------------------------------
class IActionPerformer
{
public:
	virtual ~IActionPerformer () {}
	virtual void performAction (IAction* action) = 0;

	virtual void performColorChange (UTF8StringPtr colorName, const CColor& newColor, bool remove = false) = 0;
	virtual void performTagChange (UTF8StringPtr tagName, UTF8StringPtr tagString, bool remove = false) = 0;
	virtual void performBitmapChange (UTF8StringPtr bitmapName, UTF8StringPtr bitmapPath, bool remove = false) = 0;
	virtual void performGradientChange (UTF8StringPtr gradientName, CGradient* newGradient, bool remove = false) = 0;
	virtual void performFontChange (UTF8StringPtr fontName, CFontRef newFont, bool remove = false) = 0;

	virtual void performColorNameChange (UTF8StringPtr oldName, UTF8StringPtr newName) = 0;
	virtual void performTagNameChange (UTF8StringPtr oldName, UTF8StringPtr newName) = 0;
	virtual void performFontNameChange (UTF8StringPtr oldName, UTF8StringPtr newName) = 0;
	virtual void performBitmapNameChange (UTF8StringPtr oldName, UTF8StringPtr newName) = 0;
	virtual void performGradientNameChange (UTF8StringPtr oldName, UTF8StringPtr newName) = 0;

	virtual void performAlternativeFontChange (UTF8StringPtr fontName, UTF8StringPtr newAlternativeFonts) = 0;

	virtual void performBitmapNinePartTiledChange (UTF8StringPtr bitmapName, const CRect* offsets) = 0;
	virtual void performBitmapFiltersChange (UTF8StringPtr bitmapName, const std::list<SharedPointer<UIAttributes> >& filterDescription) = 0;

	virtual void beginLiveColorChange (UTF8StringPtr colorName) = 0;
	virtual void performLiveColorChange (UTF8StringPtr colorName, const CColor& newColor) = 0;
	virtual void endLiveColorChange (UTF8StringPtr colorName) = 0;

	virtual void performTemplateNameChange (UTF8StringPtr oldName, UTF8StringPtr newName) = 0;
	virtual void performCreateNewTemplate (UTF8StringPtr name, UTF8StringPtr baseViewClassName) = 0;
	virtual void performDeleteTemplate (UTF8StringPtr name) = 0;
	virtual void performDuplicateTemplate (UTF8StringPtr name, UTF8StringPtr dupName) = 0;
	virtual void onTemplateCreation (UTF8StringPtr name, CView* view) = 0;
	virtual void onTemplateNameChange (UTF8StringPtr oldName, UTF8StringPtr newName) = 0;

	virtual void beginGroupAction (UTF8StringPtr name) = 0;
	virtual void finishGroupAction () = 0;
};

} // namespace

#endif // VSTGUI_LIVE_EDITING

#endif // __iaction__
