// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../lib/vstguifwd.h"
#include "../lib/cfont.h"
#include "../lib/cstring.h"
#include <string>
#include <list>

namespace VSTGUI {

class IController;
class IViewFactory;

//-----------------------------------------------------------------------------
class IUIDescription
{
public:
	virtual ~IUIDescription () noexcept = default;

	virtual CView* createView (UTF8StringPtr name, IController* controller) const = 0;

	virtual CBitmap* getBitmap (UTF8StringPtr name) const = 0;
	virtual CFontRef getFont (UTF8StringPtr name) const = 0;
	virtual bool getColor (UTF8StringPtr name, CColor& color) const = 0;
	virtual CGradient* getGradient (UTF8StringPtr name) const = 0;
	virtual int32_t getTagForName (UTF8StringPtr name) const = 0;
	virtual IControlListener* getControlListener (UTF8StringPtr name) const = 0;
	virtual IController* getController () const = 0;

	virtual UTF8StringPtr lookupColorName (const CColor& color) const = 0;
	virtual UTF8StringPtr lookupFontName (const CFontRef font) const = 0;
	virtual UTF8StringPtr lookupBitmapName (const CBitmap* bitmap) const = 0;
	virtual UTF8StringPtr lookupGradientName (const CGradient* gradient) const = 0;
	virtual UTF8StringPtr lookupControlTagName (const int32_t tag) const = 0;

	virtual bool getVariable (UTF8StringPtr name, double& value) const = 0;
	virtual bool getVariable (UTF8StringPtr name, std::string& value) const = 0;

	virtual void collectTemplateViewNames (std::list<const std::string*>& names) const = 0;
	virtual void collectColorNames (std::list<const std::string*>& names) const = 0;
	virtual void collectFontNames (std::list<const std::string*>& names) const = 0;
	virtual void collectBitmapNames (std::list<const std::string*>& names) const = 0;
	virtual void collectGradientNames (std::list<const std::string*>& names) const = 0;
	virtual void collectControlTagNames (std::list<const std::string*>& names) const = 0;

	virtual const IViewFactory* getViewFactory () const = 0;

	static IdStringPtr kCustomViewName;
};

//-----------------------------------------------------------------------------
struct FocusDrawingSettings
{
	bool enabled {false};
	CCoord width {1};
	UTF8String colorName;
	
	bool operator!= (const FocusDrawingSettings& o) const
	{
		return (enabled != o.enabled || width != o.width || colorName != o.colorName);
	}
};

} // VSTGUI VSTGUI
