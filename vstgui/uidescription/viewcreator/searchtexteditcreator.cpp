// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "searchtexteditcreator.h"

#include "../../lib/controls/csearchtextedit.h"
#include "../detail/uiviewcreatorattributes.h"
#include "../uiattributes.h"
#include "../uiviewcreator.h"
#include "../uiviewfactory.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace UIViewCreator {

//------------------------------------------------------------------------
CSearchTextEditCreator::CSearchTextEditCreator ()
{
	UIViewFactory::registerViewCreator (*this);
}

//------------------------------------------------------------------------
IdStringPtr CSearchTextEditCreator::getViewName () const
{
	return kCSearchTextEdit;
}

//------------------------------------------------------------------------
IdStringPtr CSearchTextEditCreator::getBaseViewName () const
{
	return kCTextEdit;
}

//------------------------------------------------------------------------
UTF8StringPtr CSearchTextEditCreator::getDisplayName () const
{
	return "Search Text Edit";
}

//------------------------------------------------------------------------
CView* CSearchTextEditCreator::create (const UIAttributes& attributes,
                                       const IUIDescription* description) const
{
	return new CSearchTextEdit (CRect (0, 0, 100, 20), nullptr, -1);
}

//------------------------------------------------------------------------
bool CSearchTextEditCreator::apply (CView* view, const UIAttributes& attributes,
                                    const IUIDescription* description) const
{
	auto ste = dynamic_cast<CSearchTextEdit*> (view);
	if (!ste)
		return false;
	CPoint p;
	if (attributes.getPointAttribute (kAttrClearMarkInset, p))
		ste->setClearMarkInset (p);
	return true;
}

//------------------------------------------------------------------------
bool CSearchTextEditCreator::getAttributeNames (std::list<std::string>& attributeNames) const
{
	attributeNames.emplace_back (kAttrClearMarkInset);
	return true;
}

//------------------------------------------------------------------------
auto CSearchTextEditCreator::getAttributeType (const std::string& attributeName) const -> AttrType
{
	if (attributeName == kAttrClearMarkInset)
		return kPointType;
	return kUnknownType;
}

//------------------------------------------------------------------------
bool CSearchTextEditCreator::getAttributeValue (CView* view, const std::string& attributeName,
                                                std::string& stringValue,
                                                const IUIDescription* desc) const
{
	auto ste = dynamic_cast<CSearchTextEdit*> (view);
	if (!ste)
		return false;
	if (attributeName == kAttrClearMarkInset)
	{
		stringValue = UIAttributes::pointToString (ste->getClearMarkInset ());
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
} // UIViewCreator
} // VSTGUI
