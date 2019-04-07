// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../iviewcreator.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace UIViewCreator {

//-----------------------------------------------------------------------------
struct IMultiBitmapControlCreator
{
	static bool apply (CView* view, const UIAttributes& attributes,
	                   const IUIDescription* description);
	static bool getAttributeNames (std::list<std::string>& attributeNames);
	static IViewCreator::AttrType getAttributeType (const std::string& attributeName);
	static bool getAttributeValue (CView* view, const std::string& attributeName,
	                               std::string& stringValue, const IUIDescription* desc);
};

//-----------------------------------------------------------------------------
class MultiBitmapControlCreator : public ViewCreatorAdapter
{
public:
	bool getAttributeNames (std::list<std::string>& attributeNames) const override
	{
		return IMultiBitmapControlCreator::getAttributeNames (attributeNames);
	}
	AttrType getAttributeType (const std::string& attributeName) const override
	{
		return IMultiBitmapControlCreator::getAttributeType (attributeName);
	}
	bool apply (CView* view, const UIAttributes& attributes,
	            const IUIDescription* description) const override
	{
		return IMultiBitmapControlCreator::apply (view, attributes, description);
	}
	bool getAttributeValue (CView* view, const std::string& attributeName, std::string& stringValue,
	                        const IUIDescription* desc) const override
	{
		return IMultiBitmapControlCreator::getAttributeValue (view, attributeName, stringValue,
		                                                      desc);
	}
};

//------------------------------------------------------------------------
} // UIViewCreator
} // VSTGUI
