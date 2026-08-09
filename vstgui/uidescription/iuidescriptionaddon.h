// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "iuidescription.h"

//------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------
struct IUIDescriptionAddOn
{
	virtual ~IUIDescriptionAddOn () noexcept = default;

	/** called after the desc was parsed */
	virtual void afterParsing (const IUIDescription& desc) = 0;
	/** called before the desc is saved */
	virtual void beforeSaving (const IUIDescription& desc) = 0;
	/** called when the desc is going to be destroyed */
	virtual void onDestroy (const IUIDescription& desc) = 0;

	using CreateTemplateViewFunc =
		std::function<SPtr<CView> (UTF8StringPtr name, const SPtr<IController>& controller)>;
	/** called when a new template view should be created. The provided CreateTemplateViewFunc can
	 *	be used to create the view in a normal way
	 */
	virtual CreateTemplateViewFunc onCreateTemplateView (const IUIDescription& desc,
														 const CreateTemplateViewFunc& f) = 0;

	/** the add-on can wrap the view factory or return the original one if not needed */
	virtual SPtr<IViewFactory> getViewFactory (const IUIDescription& desc,
											   const SPtr<IViewFactory>& originalFactory) = 0;

	/** called when the desc is going into edit mode */
	virtual void onEditingStart (const IUIDescription& desc) = 0;
	/** called when the desc is going out of edit mode */
	virtual void onEditingEnd (const IUIDescription& desc) = 0;
};

//------------------------------------------------------------------------
struct UIDescriptionAddOnAdapter : IUIDescriptionAddOn
{
	void afterParsing (const IUIDescription& desc) override {}
	void beforeSaving (const IUIDescription& desc) override {}
	void onDestroy (const IUIDescription& desc) override {}
	CreateTemplateViewFunc onCreateTemplateView (const IUIDescription& desc,
												 const CreateTemplateViewFunc& f) override
	{
		return [=] (UTF8StringPtr name, const SPtr<IController>& controller) {
			return f (name, controller);
		};
	}
	SPtr<IViewFactory> getViewFactory (const IUIDescription& desc,
									   const SPtr<IViewFactory>& originalFactory) override
	{
		return originalFactory;
	}

	void onEditingStart (const IUIDescription& desc) override {}
	void onEditingEnd (const IUIDescription& desc) override {}
};

//------------------------------------------------------------------------
} // VSTGUI
