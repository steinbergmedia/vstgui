// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../uidescription/icontroller.h"
#include "../uidescription/iviewfactory.h"
#include "../uidescription/iuidescription.h"
#include "../uidescription/iuidescriptionaddon.h"

namespace VSTGUI {

//------------------------------------------------------------------------
class UIScripting : public UIDescriptionAddOnAdapter
{
public:
	using OnScriptException = std::function<void (const std::string& reason)>;

	static void init (const OnScriptException& func = {});

	~UIScripting () noexcept;

private:
	UIScripting ();

	void afterParsing (IUIDescription* desc) override;
	void beforeSaving (IUIDescription* desc) override;
	void onDestroy (IUIDescription* desc) override;
	CreateTemplateViewFunc onCreateTemplateView (const IUIDescription* desc,
												 const CreateTemplateViewFunc& f) override;
	IViewFactory* getViewFactory (IUIDescription* desc, IViewFactory* originalFactory) override;
	void onEditingStart (IUIDescription* desc) override;
	void onEditingEnd (IUIDescription* desc) override;

	struct Impl;
	std::unique_ptr<Impl> impl;

	friend std::unique_ptr<UIScripting> std::make_unique<UIScripting> ();
};

//------------------------------------------------------------------------
} // VSTGUI
