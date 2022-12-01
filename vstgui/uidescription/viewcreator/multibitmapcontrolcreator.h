// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../iviewcreator.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace UIViewCreator {

#if VSTGUI_ENABLE_DEPRECATED_METHODS
//------------------------------------------------------------------------
struct IMultiBitmapControlCreator
{
	using string = IViewCreator::string;
	using StringList = IViewCreator::StringList;

	static bool apply (CView* view, const UIAttributes& attributes,
	                   const IUIDescription* description);
	static bool getAttributeNames (StringList& attributeNames);
	static IViewCreator::AttrType getAttributeType (const string& attributeName);
	static bool getAttributeValue (CView* view, const string& attributeName, string& stringValue,
	                               const IUIDescription* desc);
};

//------------------------------------------------------------------------
struct MultiBitmapControlCreator : ViewCreatorAdapter
{
	bool getAttributeNames (StringList& attributeNames) const override
	{
		return IMultiBitmapControlCreator::getAttributeNames (attributeNames);
	}
	AttrType getAttributeType (const string& attributeName) const override
	{
		return IMultiBitmapControlCreator::getAttributeType (attributeName);
	}
	bool apply (CView* view, const UIAttributes& attributes,
	            const IUIDescription* description) const override
	{
		return IMultiBitmapControlCreator::apply (view, attributes, description);
	}
	bool getAttributeValue (CView* view, const string& attributeName, string& stringValue,
	                        const IUIDescription* desc) const override
	{
		return IMultiBitmapControlCreator::getAttributeValue (view, attributeName, stringValue,
		                                                      desc);
	}
};
#else
using MultiBitmapControlCreator = ViewCreatorAdapter;

#endif // VSTGUI_ENABLE_DEPRECATED_METHODS

//------------------------------------------------------------------------
} // UIViewCreator
} // VSTGUI
