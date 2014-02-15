//
//  iviewcreator.h
//  vstgui
//
//  Created by Arne Scheffler on 15/02/14.
//
//

#ifndef __iviewcreator__
#define __iviewcreator__

#include "../lib/vstguibase.h"
#include <string>
#include <list>

namespace VSTGUI {
class CView;
class IUIDescription;
class UIAttributes;

//-----------------------------------------------------------------------------
/// @brief View creator interface
///	@ingroup new_in_4_0
//-----------------------------------------------------------------------------
class IViewCreator
{
public:
	virtual ~IViewCreator () {}
	
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
		kListType
	};

	virtual IdStringPtr getViewName () const = 0;
	virtual IdStringPtr getBaseViewName () const = 0;
	virtual CView* create (const UIAttributes& attributes, IUIDescription* description) const = 0;
	virtual bool apply (CView* view, const UIAttributes& attributes, IUIDescription* description) const = 0;
	virtual bool getAttributeNames (std::list<std::string>& attributeNames) const = 0;
	virtual AttrType getAttributeType (const std::string& attributeName) const = 0;
	virtual bool getAttributeValue (CView* view, const std::string& attributeName, std::string& stringValue, IUIDescription* desc) const = 0;
	// list type support
	virtual bool getPossibleListValues (const std::string& attributeName, std::list<const std::string*>& values) const { return false; }
};

} // namespace

#endif // __iviewcreator__
