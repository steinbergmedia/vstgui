// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __uidescriptionlistener__
#define __uidescriptionlistener__

#include "../lib/vstguibase.h"
#include "uidescriptionfwd.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
class UIDescriptionListener
{
public:
	virtual ~UIDescriptionListener () noexcept {};

	virtual bool doUIDescTemplateUpdate (UIDescription* desc, UTF8StringPtr name) = 0;
	virtual void onUIDescTagChanged (UIDescription* desc) = 0;
	virtual void onUIDescColorChanged (UIDescription* desc) = 0;
	virtual void onUIDescFontChanged (UIDescription* desc) = 0;
	virtual void onUIDescBitmapChanged (UIDescription* desc) = 0;
	virtual void onUIDescTemplateChanged (UIDescription* desc) = 0;
	virtual void onUIDescGradientChanged (UIDescription* desc) = 0;
	virtual void beforeUIDescSave (UIDescription* desc) = 0;
};

//-----------------------------------------------------------------------------
class UIDescriptionListenerAdapter : public UIDescriptionListener
{
public:
	bool doUIDescTemplateUpdate (UIDescription* desc, UTF8StringPtr name) override { return true; };
	void onUIDescTagChanged (UIDescription* desc) override {};
	void onUIDescColorChanged (UIDescription* desc) override {};
	void onUIDescFontChanged (UIDescription* desc) override {};
	void onUIDescBitmapChanged (UIDescription* desc) override {};
	void onUIDescTemplateChanged (UIDescription* desc) override {};
	void onUIDescGradientChanged (UIDescription* desc) override {};
	void beforeUIDescSave (UIDescription* desc) override {};
};

} // VSTGUI

#endif // __uidescriptionlistener__
