#pragma once

#include "../../../uidescription/iuidescription.h"

namespace VSTGUI {

class UIDescriptionAdapter : public IUIDescription
{
public:
	CView* createView (UTF8StringPtr name, IController* controller) const override { return nullptr; }
	CBitmap* getBitmap (UTF8StringPtr name) const override { return nullptr; }
	CFontRef getFont (UTF8StringPtr name) const override { return nullptr; }
	bool getColor (UTF8StringPtr name, CColor& color) const override { return false; }
	CGradient* getGradient (UTF8StringPtr name) const override { return nullptr; }
	int32_t getTagForName (UTF8StringPtr name) const override { return -1; }
	IControlListener* getControlListener (UTF8StringPtr name) const override { return nullptr; }
	IController* getController () const override { return nullptr; }

	UTF8StringPtr lookupColorName (const CColor& color) const override { return nullptr; }
	UTF8StringPtr lookupFontName (const CFontRef font) const override { return nullptr; }
	UTF8StringPtr lookupBitmapName (const CBitmap* bitmap) const override { return nullptr; }
	UTF8StringPtr lookupGradientName (const CGradient* gradient) const override { return nullptr; }
	UTF8StringPtr lookupControlTagName (const int32_t tag) const override { return nullptr; }

	bool getVariable (UTF8StringPtr name, double& value) const override { return false; }
	bool getVariable (UTF8StringPtr name, std::string& value) const override { return false; }

	void collectTemplateViewNames (std::list<const std::string*>& names) const override {}
	void collectColorNames (std::list<const std::string*>& names) const override {}
	void collectFontNames (std::list<const std::string*>& names) const override {}
	void collectBitmapNames (std::list<const std::string*>& names) const override {}
	void collectGradientNames (std::list<const std::string*>& names) const override {}
	void collectControlTagNames (std::list<const std::string*>& names) const override {}

	const IViewFactory* getViewFactory () const override { return nullptr; }
};

}
