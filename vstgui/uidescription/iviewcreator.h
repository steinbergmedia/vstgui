// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../lib/vstguifwd.h"
#include <list>
#include <string>

namespace VSTGUI {
class IUIDescription;
class UIAttributes;

//-----------------------------------------------------------------------------
/// @brief View creator interface
///	@ingroup new_in_4_0
//-----------------------------------------------------------------------------
class IViewCreator
{
public:
	using string = std::string;
	using StringList = std::list<string>;
	using ConstStringPtrList = std::list<const string*>;

	virtual ~IViewCreator () noexcept = default;

	enum AttrType
	{
		kUnknownType,
		kBooleanType,
		kIntegerType,
		kFloatType,
		kStringType,
		kColorType,
		kFontType,
		kBitmapType,
		kPointType,
		kRectType,
		kTagType,
		kListType,
		kGradientType
	};

	virtual IdStringPtr getViewName () const = 0;
	virtual IdStringPtr getBaseViewName () const = 0;
	virtual CView* create (const UIAttributes& attributes,
	                       const IUIDescription* description) const = 0;
	virtual bool apply (CView* view, const UIAttributes& attributes,
	                    const IUIDescription* description) const = 0;
	virtual bool getAttributeNames (StringList& attributeNames) const = 0;
	virtual AttrType getAttributeType (const string& attributeName) const = 0;
	virtual bool getAttributeValue (CView* view, const string& attributeName, string& stringValue,
	                                const IUIDescription* desc) const = 0;
	// optional list type support
	virtual bool getPossibleListValues (const string& attributeName,
	                                    ConstStringPtrList& values) const = 0;
	// optional value range
	virtual bool getAttributeValueRange (const string& attributeName, double& minValue,
	                                     double& maxValue) const = 0;
	// optional display name
	virtual UTF8StringPtr getDisplayName () const = 0;
};

//-----------------------------------------------------------------------------
/// @brief View creator interface adapter
//-----------------------------------------------------------------------------
class ViewCreatorAdapter : public IViewCreator
{
public:
	bool apply (CView* view, const UIAttributes& attributes,
	            const IUIDescription* description) const override
	{
		return true;
	}
	bool getAttributeNames (StringList& attributeNames) const override { return true; }
	AttrType getAttributeType (const string& attributeName) const override { return kUnknownType; }
	bool getAttributeValue (CView* view, const string& attributeName, string& stringValue,
	                        const IUIDescription* desc) const override
	{
		return false;
	}
	bool getPossibleListValues (const string& attributeName,
	                            ConstStringPtrList& values) const override
	{
		return false;
	}
	bool getAttributeValueRange (const string& attributeName, double& minValue,
	                             double& maxValue) const override
	{
		return false;
	}
	UTF8StringPtr getDisplayName () const override { return getViewName (); }
};

} // VSTGUI
