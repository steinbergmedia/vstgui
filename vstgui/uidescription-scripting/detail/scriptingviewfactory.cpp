// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "scriptingviewfactory.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace ScriptingInternal {

//------------------------------------------------------------------------
JavaScriptViewFactory::JavaScriptViewFactory (IScriptContextInternal* scripting,
											  IViewFactory* origFactory)
: Super (origFactory), scriptContext (scripting)
{
}

//------------------------------------------------------------------------
JavaScriptViewFactory::~JavaScriptViewFactory () noexcept
{
	std::for_each (viewControllerLinks.begin (), viewControllerLinks.end (),
				   [this] (const auto& el) {
					   el.first->unregisterViewListener (this);
					   el.second->scriptContextDestroyed (scriptContext);
				   });
}

//------------------------------------------------------------------------
CView* JavaScriptViewFactory::createView (const UIAttributes& attributes,
										  const IUIDescription* description) const
{
	if (auto view = Super::createView (attributes, description))
	{
		if (auto value = attributes.getAttributeValue (kAttrScript))
		{
			std::optional<std::string> verifiedScript;
			if (auto scriptViewController =
					dynamic_cast<IScriptControllerExtension*> (description->getController ()))
			{
				verifiedScript = scriptViewController->verifyScript (view, *value, scriptContext);
				view->registerViewListener (const_cast<JavaScriptViewFactory*> (this));
				viewControllerLinks.emplace_back (view, scriptViewController);
			}
			const auto& script = verifiedScript ? *verifiedScript : *value;
			auto scriptSize = static_cast<uint32_t> (script.size () + 1);
			view->setAttribute (scriptAttrID, scriptSize, script.data ());
			if (!disabled)
			{
				scriptContext->onViewCreated (view, script);
			}
		}
		return view;
	}
	return {};
}

//------------------------------------------------------------------------
bool JavaScriptViewFactory::getAttributeNamesForView (CView* view, StringList& attributeNames) const
{
	if (Super::getAttributeNamesForView (view, attributeNames))
	{
		attributeNames.emplace_back (kAttrScript);
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
auto JavaScriptViewFactory::getAttributeType (CView* view, const std::string& attributeName) const
	-> IViewCreator::AttrType
{
	if (attributeName == kAttrScript)
		return IViewCreator::kScriptType;
	return Super::getAttributeType (view, attributeName);
}

//------------------------------------------------------------------------
bool JavaScriptViewFactory::getAttributeValue (CView* view, const std::string& attributeName,
											   std::string& stringValue,
											   const IUIDescription* desc) const
{
	if (attributeName == kAttrScript)
	{
		uint32_t attrSize = 0;
		if (view->getAttributeSize (scriptAttrID, attrSize) && attrSize > 0)
		{
			stringValue.resize (attrSize - 1);
			if (!view->getAttribute (scriptAttrID, attrSize, stringValue.data (), attrSize))
				stringValue = "";
			return true;
		}
		return false;
	}
	return Super::getAttributeValue (view, attributeName, stringValue, desc);
}

//------------------------------------------------------------------------
bool JavaScriptViewFactory::applyAttributeValues (CView* view, const UIAttributes& attributes,
												  const IUIDescription* desc) const
{
	if (auto value = attributes.getAttributeValue (kAttrScript))
	{
		if (value->empty ())
			view->removeAttribute (scriptAttrID);
		else
			view->setAttribute (scriptAttrID, static_cast<uint32_t> (value->size () + 1),
								value->data ());
		if (!disabled)
			scriptContext->onViewCreated (view, *value);
		return true;
	}
	return Super::applyAttributeValues (view, attributes, desc);
}

//------------------------------------------------------------------------
void JavaScriptViewFactory::setScriptingDisabled (bool state) { disabled = state; }

//------------------------------------------------------------------------
void JavaScriptViewFactory::viewWillDelete (CView* view)
{
	auto it = std::find_if (viewControllerLinks.begin (), viewControllerLinks.end (),
							[view] (const auto& el) { return el.first == view; });
	if (it != viewControllerLinks.end ())
	{
		it->second->scriptContextDestroyed (scriptContext);
		viewControllerLinks.erase (it);
	}
	view->unregisterViewListener (this);
}

//------------------------------------------------------------------------
} // ScriptingInternal
} // VSTGUI
