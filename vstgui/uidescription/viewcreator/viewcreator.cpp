// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "viewcreator.h"

#include "../../lib/cview.h"
#include "../detail/uiviewcreatorattributes.h"
#include "../uiattributes.h"
#include "../uiviewcreator.h"
#include "../uiviewfactory.h"

#if VSTGUI_LIVE_EDITING
#include "../../lib/cdrawcontext.h"
#endif

//------------------------------------------------------------------------
namespace VSTGUI {
namespace UIViewCreator {

#if VSTGUI_LIVE_EDITING
//-----------------------------------------------------------------------------
class LiveEditingCView : public CView
{
public:
	LiveEditingCView (const CRect& r) : CView (r) {}
	void draw (CDrawContext* context) override
	{
		if (getDrawBackground ())
		{
			CView::draw (context);
			return;
		}
		context->setLineWidth (1.);
		context->setLineStyle (kLineSolid);
		context->setDrawMode (kAliasing);
		context->setFrameColor ({200, 200, 200, 100});
		context->setFillColor ({200, 200, 200, 100});
		constexpr auto width = 5.;
		CRect viewSize = getViewSize ();
		auto r = viewSize;
		r.setSize ({width, width});
		uint32_t row = 0u;
		while (r.top < viewSize.bottom)
		{
			uint32_t column = (row % 2) ? 0u : 1u;
			while (r.left < viewSize.right)
			{
				if (column % 2)
					context->drawRect (r, kDrawFilled);
				r.offset (width, 0);
				++column;
			}
			r.left = viewSize.left;
			r.right = r.left + width;
			r.offset (0, width);
			++row;
		}
		context->drawRect (viewSize, kDrawStroked);
		setDirty (false);
	}
};
using SimpleCView = LiveEditingCView;
#else
using SimpleCView = CView;
#endif

//------------------------------------------------------------------------
ViewCreator::ViewCreator ()
{
	UIViewFactory::registerViewCreator (*this);
}

//------------------------------------------------------------------------
IdStringPtr ViewCreator::getViewName () const
{
	return kCView;
}

//------------------------------------------------------------------------
IdStringPtr ViewCreator::getBaseViewName () const
{
	return nullptr;
}

//------------------------------------------------------------------------
UTF8StringPtr ViewCreator::getDisplayName () const
{
	return "View";
}

//------------------------------------------------------------------------
CView* ViewCreator::create (const UIAttributes& attributes, const IUIDescription* description) const
{
	return new SimpleCView (CRect (0, 0, 50, 50));
}

//------------------------------------------------------------------------
bool ViewCreator::apply (CView* view, const UIAttributes& attributes,
                         const IUIDescription* description) const
{
	CPoint p;
	CRect size;
	if (attributes.getPointAttribute (kAttrOrigin, p))
	{
		CRect origViewSize = view->getViewSize ();
		size.setTopLeft (p);
		size.setWidth (origViewSize.getWidth ());
		size.setHeight (origViewSize.getHeight ());
		view->setViewSize (size, false);
		view->setMouseableArea (size);
	}
	if (attributes.getPointAttribute (kAttrSize, p))
	{
		size = view->getViewSize ();
		size.setSize (p);
		view->setViewSize (size, false);
		view->setMouseableArea (size);
	}

	CBitmap* bitmap;
	if (stringToBitmap (attributes.getAttributeValue (kAttrBitmap), bitmap, description))
		view->setBackground (bitmap);
	if (stringToBitmap (attributes.getAttributeValue (kAttrDisabledBitmap), bitmap, description))
		view->setDisabledBackground (bitmap);

	bool b;
	if (attributes.getBooleanAttribute (kAttrTransparent, b))
		view->setTransparency (b);
	if (attributes.getBooleanAttribute (kAttrMouseEnabled, b))
		view->setMouseEnabled (b);
	if (attributes.hasAttribute (kAttrWantsFocus) &&
	    attributes.getBooleanAttribute (kAttrWantsFocus, b))
		view->setWantsFocus (b);

	const auto* autosizeAttr = attributes.getAttributeValue (kAttrAutosize);
	if (autosizeAttr)
	{
		int32_t autosize = kAutosizeNone;
		if (autosizeAttr->find ("left") != string::npos)
			autosize |= kAutosizeLeft;
		if (autosizeAttr->find ("top") != string::npos)
			autosize |= kAutosizeTop;
		if (autosizeAttr->find ("right") != string::npos)
			autosize |= kAutosizeRight;
		if (autosizeAttr->find ("bottom") != string::npos)
			autosize |= kAutosizeBottom;
		if (autosizeAttr->find ("row") != string::npos)
			autosize |= kAutosizeRow;
		if (autosizeAttr->find ("column") != string::npos)
			autosize |= kAutosizeColumn;
		view->setAutosizeFlags (autosize);
	}
	const auto* tooltipAttr = attributes.getAttributeValue (kAttrTooltip);
	if (tooltipAttr)
	{
		if (!tooltipAttr->empty ())
			view->setTooltipText (tooltipAttr->data ());
		else
			view->setTooltipText (nullptr);
	}

	const auto* customViewAttr = attributes.getAttributeValue (kAttrCustomViewName);
	if (customViewAttr)
		view->setAttribute ('uicv', static_cast<uint32_t> (customViewAttr->size () + 1),
		                    customViewAttr->c_str ());

	const auto* subControllerAttr = attributes.getAttributeValue (kAttrSubController);
	if (subControllerAttr)
		view->setAttribute ('uisc', static_cast<uint32_t> (subControllerAttr->size () + 1),
		                    subControllerAttr->c_str ());

	double opacity;
	if (attributes.getDoubleAttribute (kAttrOpacity, opacity))
		view->setAlphaValue (static_cast<float> (opacity));

	return true;
}

//------------------------------------------------------------------------
bool ViewCreator::getAttributeNames (StringList& attributeNames) const
{
	attributeNames.emplace_back (kAttrOrigin);
	attributeNames.emplace_back (kAttrSize);
	attributeNames.emplace_back (kAttrOpacity);
	attributeNames.emplace_back (kAttrTransparent);
	attributeNames.emplace_back (kAttrMouseEnabled);
	attributeNames.emplace_back (kAttrWantsFocus);
	attributeNames.emplace_back (kAttrBitmap);
	attributeNames.emplace_back (kAttrDisabledBitmap);
	attributeNames.emplace_back (kAttrAutosize);
	attributeNames.emplace_back (kAttrTooltip);
	attributeNames.emplace_back (kAttrCustomViewName);
	attributeNames.emplace_back (kAttrSubController);
	return true;
}

//------------------------------------------------------------------------
auto ViewCreator::getAttributeType (const string& attributeName) const -> AttrType
{
	if (attributeName == kAttrOrigin)
		return kPointType;
	else if (attributeName == kAttrSize)
		return kPointType;
	else if (attributeName == kAttrOpacity)
		return kFloatType;
	else if (attributeName == kAttrTransparent)
		return kBooleanType;
	else if (attributeName == kAttrMouseEnabled)
		return kBooleanType;
	else if (attributeName == kAttrWantsFocus)
		return kBooleanType;
	else if (attributeName == kAttrBitmap)
		return kBitmapType;
	else if (attributeName == kAttrDisabledBitmap)
		return kBitmapType;
	else if (attributeName == kAttrAutosize)
		return kStringType;
	else if (attributeName == kAttrTooltip)
		return kStringType;
	else if (attributeName == kAttrCustomViewName)
		return kStringType;
	else if (attributeName == kAttrSubController)
		return kStringType;
	return kUnknownType;
}

//------------------------------------------------------------------------
bool ViewCreator::getAttributeValue (CView* view, const string& attributeName, string& stringValue,
                                     const IUIDescription* desc) const
{
	if (attributeName == kAttrOrigin)
	{
		stringValue = UIAttributes::pointToString (view->getViewSize ().getTopLeft ());
		return true;
	}
	else if (attributeName == kAttrSize)
	{
		stringValue = UIAttributes::pointToString (view->getViewSize ().getSize ());
		return true;
	}
	else if (attributeName == kAttrOpacity)
	{
		stringValue = UIAttributes::doubleToString (view->getAlphaValue ());
		return true;
	}
	else if (attributeName == kAttrTransparent)
	{
		stringValue = view->getTransparency () ? strTrue : strFalse;
		return true;
	}
	else if (attributeName == kAttrMouseEnabled)
	{
		stringValue = view->getMouseEnabled () ? strTrue : strFalse;
		return true;
	}
	else if (attributeName == kAttrWantsFocus)
	{
		stringValue = view->wantsFocus () ? strTrue : strFalse;
		return true;
	}
	else if (attributeName == kAttrBitmap)
	{
		CBitmap* bitmap = view->getBackground ();
		if (bitmap)
			bitmapToString (bitmap, stringValue, desc);
		else
			stringValue = "";
		return true;
	}
	else if (attributeName == kAttrDisabledBitmap)
	{
		CBitmap* bitmap = view->getDisabledBackground ();
		if (bitmap)
			bitmapToString (bitmap, stringValue, desc);
		else
			stringValue = "";
		return true;
	}
	else if (attributeName == kAttrAutosize)
	{
		std::stringstream stream;
		int32_t autosize = view->getAutosizeFlags ();
		if (autosize == 0)
			return true;
		if (autosize & kAutosizeLeft)
			stream << "left ";
		if (autosize & kAutosizeRight)
			stream << "right ";
		if (autosize & kAutosizeTop)
			stream << "top ";
		if (autosize & kAutosizeBottom)
			stream << "bottom ";
		if (autosize & kAutosizeRow)
			stream << "row ";
		if (autosize & kAutosizeColumn)
			stream << "column ";
		stringValue = stream.str ();
		return true;
	}
	else if (attributeName == kAttrTooltip)
	{
		return getViewAttributeString (view, kCViewTooltipAttribute, stringValue);
	}
	else if (attributeName == kAttrCustomViewName)
	{
		return getViewAttributeString (view, 'uicv', stringValue);
	}
	else if (attributeName == kAttrSubController)
	{
		return getViewAttributeString (view, 'uisc', stringValue);
	}
	return false;
}

//------------------------------------------------------------------------
bool ViewCreator::getAttributeValueRange (const string& attributeName, double& minValue,
                                          double& maxValue) const
{
	if (attributeName == kAttrOpacity)
	{
		minValue = 0.;
		maxValue = 1.;
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
bool ViewCreator::getViewAttributeString (CView* view, const CViewAttributeID attrID, string& value)
{
	uint32_t attrSize = 0;
	if (view->getAttributeSize (attrID, attrSize))
	{
		char* cstr = new char[attrSize + 1];
		if (view->getAttribute (attrID, attrSize, cstr, attrSize))
			value = cstr;
		else
			value = "";
		delete[] cstr;
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
} // UIViewCreator
} // VSTGUI
