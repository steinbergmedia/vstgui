// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "iscriptcontextinternal.h"
#include "../../uidescription/uiattributes.h"
#include "../../uidescription/uiviewfactory.h"
#include "../../lib/iviewlistener.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace ScriptingInternal {

//------------------------------------------------------------------------
struct JavaScriptViewFactory : ViewFactoryDelegate,
							   ViewListenerAdapter
{
	static constexpr CViewAttributeID scriptAttrID = 'scri';

	JavaScriptViewFactory (ScriptingInternal::IScriptContextInternal* scripting,
						   IViewFactory* origFactory);
	~JavaScriptViewFactory () noexcept;

	CView* createView (const UIAttributes& attributes,
					   const IUIDescription* description) const override;
	bool getAttributeNamesForView (CView* view, StringList& attributeNames) const override;
	IViewCreator::AttrType getAttributeType (CView* view,
											 const std::string& attributeName) const override;
	bool getAttributeValue (CView* view, const std::string& attributeName, std::string& stringValue,
							const IUIDescription* desc) const override;
	bool applyAttributeValues (CView* view, const UIAttributes& attributes,
							   const IUIDescription* desc) const override;
	void setScriptingDisabled (bool state);

private:
	void viewWillDelete (CView* view) override;

	using Super = ViewFactoryDelegate;
	using ViewControllerLink = std::pair<CView*, IScriptControllerExtension*>;
	using ViewControllerLinkVector = std::vector<ViewControllerLink>;

	ScriptingInternal::IScriptContextInternal* scriptContext;
	mutable ViewControllerLinkVector viewControllerLinks;
	bool disabled {false};
};

//------------------------------------------------------------------------
} // ScriptingInternal
} // VSTGUI
