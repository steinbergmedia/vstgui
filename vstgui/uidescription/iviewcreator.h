// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __iviewcreator__
#define __iviewcreator__

#include "../lib/vstguifwd.h"
#include <string>
#include <list>

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
	virtual ~IViewCreator () noexcept = default;
	
	enum AttrType {
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
	virtual CView* create (const UIAttributes& attributes, const IUIDescription* description) const = 0;
	virtual bool apply (CView* view, const UIAttributes& attributes, const IUIDescription* description) const = 0;
	virtual bool getAttributeNames (std::list<std::string>& attributeNames) const = 0;
	virtual AttrType getAttributeType (const std::string& attributeName) const = 0;
	virtual bool getAttributeValue (CView* view, const std::string& attributeName, std::string& stringValue, const IUIDescription* desc) const = 0;
	// optional list type support
	virtual bool getPossibleListValues (const std::string& attributeName, std::list<const std::string*>& values) const = 0;
	// optional value range
	virtual bool getAttributeValueRange (const std::string& attributeName, double& minValue, double &maxValue) const = 0;
	// optional display name
	virtual UTF8StringPtr getDisplayName () const = 0;
};

//-----------------------------------------------------------------------------
/// @brief View creator interface adapter
//-----------------------------------------------------------------------------
class ViewCreatorAdapter : public IViewCreator
{
public:
	bool apply (CView* view, const UIAttributes& attributes, const IUIDescription* description) const override { return true; }
	bool getAttributeNames (std::list<std::string>& attributeNames) const override { return true; }
	AttrType getAttributeType (const std::string& attributeName) const override { return kUnknownType; }
	bool getAttributeValue (CView* view, const std::string& attributeName, std::string& stringValue, const IUIDescription* desc) const override { return false; }
	bool getPossibleListValues (const std::string& attributeName, std::list<const std::string*>& values) const override { return false; }
	bool getAttributeValueRange (const std::string& attributeName, double& minValue, double &maxValue) const override { return false; }
	UTF8StringPtr getDisplayName () const override { return getViewName (); }
};

} // namespace

#endif // __iviewcreator__
