//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 4.2
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2013, Steinberg Media Technologies, All Rights Reserved
//-----------------------------------------------------------------------------
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
// 
//   * Redistributions of source code must retain the above copyright notice, 
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation 
//     and/or other materials provided with the distribution.
//   * Neither the name of the Steinberg Media Technologies nor the names of its
//     contributors may be used to endorse or promote products derived from this 
//     software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

/**
@page uidescription_attributes UI XML Attribute Definitions

@section viewclasses View Classes and their attributes
- @ref cview @n
- @ref cviewcontainer @n
- @ref ccontrol @n
- @ref conoffbutton @n
- @ref cparamdisplay @n
- @ref ctextlabel @n
- @ref ctextedit @n
- @ref cknob @n
- @ref cverticalswitch @n
- @ref chorizontalswitch @n
- @ref crockerswitch @n
- @ref cmoviebitmap @n
- @ref ckickbutton @n
- @ref cslider @n

@section cview CView
Declaration:
@verbatim <view class="CView" /> @endverbatim

Attributes:
- \b origin [Point]
- \b size [Point]
- \b transparent [true/false]
- \b bitmap [bitmap name]
- \b autosize [combination of <i>left</i>, <i>top</i>, <i>right</i>, <i>bottom</i>, <i>row</i>, or <i>column</i> see VSTGUI::CViewAutosizing]
- \b tooltip [tooltip text]

@section cviewcontainer CViewContainer
Declaration:
@verbatim <view class="CViewContainer" /> @endverbatim

Inherites attributes from @ref cview @n

Attributes:
- \b background-color [color]

@section ccontrol CControl
A CControl is an abstract class and can not be created directly

Inherites attributes from @ref cview @n

Attributes:
- \b control-tag [tag name]
- \b default-value [float]
- \b min-value [float]
- \b max-value [float]
- \b wheel-inc-value [float]
- \b background-offset [Point]

@section conoffbutton COnOffButton
Declaration:
@verbatim <view class="COnOffButton" /> @endverbatim

Inherites attributes from @ref ccontrol @n

@section cparamdisplay CParamDisplay
Declaration:
@verbatim <view class="CParamDisplay" /> @endverbatim

Inherites attributes from @ref ccontrol @n

Attributes:
- \b font [font name]
- \b font-color [color]
- \b back-color [color]
- \b frame-color [color]
- \b shadow-color [color]
- \b font-antialias [true/false]
- \b style-3D-in [true/false]
- \b style-3D-out [true/false]
- \b style-no-frame [true/false]
- \b style-no-text [true/false]
- \b style-no-draw [true/false]
- \b style-shadow-text [true/false]
- \b text-alignment [left/center/right]

@section ctextlabel CTextLabel
Declaration:
@verbatim <view class="CTextLabel" /> @endverbatim

Inherites attributes from @ref cparamdisplay @n

Attributes:
- \b title [string]

@section ctextedit CTextEdit
Declaration:
@verbatim <view class="CTextEdit" /> @endverbatim

Inherites attributes from @ref cparamdisplay @n

Attributes:
- \b title [string]

@section cknob CKnob
Declaration:
@verbatim <view class="CKnob" /> @endverbatim

Inherites attributes from @ref ccontrol @n

Attributes:
- \b angle-start [float]
- \b angle-range [float]
- \b value-inset [int]
- \b zoom-factor [float]
- \b handle-shadow-color [color]
- \b handle-color [color]
- \b handle-bitmap [bitmap name]

@section canimknob CAnimKnob
Declaration:
@verbatim <view class="CAnimKnob" /> @endverbatim

Inherites attributes from @ref cknob @n

Attributes:
- \b height-of-one-image [int]

@section cverticalswitch CVerticalSwitch
Declaration:
@verbatim <view class="CVerticalSwitch" /> @endverbatim

Inherites attributes from @ref ccontrol @n

Attributes:
- \b height-of-one-image [int]

@section chorizontalswitch CHorizontalSwitch
Declaration:
@verbatim <view class="CHorizontalSwitch" /> @endverbatim

Inherites attributes from @ref ccontrol @n

Attributes:
- \b height-of-one-image [int]

@section crockerswitch CRockerSwitch
Declaration:
@verbatim <view class="CRockerSwitch" /> @endverbatim

Inherites attributes from @ref ccontrol @n

Attributes:
- \b height-of-one-image [int]

@section cmoviebitmap CMovieBitmap
Declaration:
@verbatim <view class="CMovieBitmap" /> @endverbatim

Inherites attributes from @ref ccontrol @n

Attributes:
- \b height-of-one-image [int]

@section cmoviebutton CMovieButton
Declaration:
@verbatim <view class="CMovieButton" /> @endverbatim

Inherites attributes from @ref ccontrol @n

Attributes:
- \b height-of-one-image [int]

@section ckickbutton CKickButton
Declaration:
@verbatim <view class="CKickButton" /> @endverbatim

Inherites attributes from @ref ccontrol @n

Attributes:
- \b height-of-one-image [int]

@section cslider CSlider
Declaration:
@verbatim <view class="CSlider" /> @endverbatim

Inherites attributes from @ref ccontrol @n

Attributes:
- \b transparent-handle [true/false]
- \b free-click [true/false]
- \b handle-bitmap [bitmap name
- \b handle-offset [Point]
- \b bitmap-offset [Point]
- \b zoom-factor [float]
- \b orientation [vertical/horizontal]
- \b reverse-orientation [true/false]

@cond ignore
*/

/*
class CViewCreator : public IViewCreator
{
public:
	IdStringPtr getViewName () const { return "CView"; }
	IdStringPtr getBaseViewName () const { return 0; }
	CView* create (const UIAttributes& attributes, IUIDescription* description) const { return new CView (CRect (0, 0, 0, 0)); }
	bool apply (CView* view, const UIAttributes& attributes, IUIDescription* description) const
	{
		CControl* control = dynamic_cast<CControl*> (view);
		if (control == 0)
			return false;
		return true;
	}
	bool getAttributeNames (std::list<std::string>& attributeNames) const
	{
		attributeNames.push_back ("empty");
		return true;
	}
	AttrType getAttributeType (const std::string& attributeName) const
	{
		if (attributeName == "empty") return kUnknownType;
		else if (attributeName == "empty2") return kUnknownType;
		return kUnknownType;
	}

};
*/

#include "uiviewfactory.h"
#include "uiviewcreator.h"
#include "uiviewswitchcontainer.h"
#include "../vstgui.h"
#include <sstream>

#if WINDOWS
#define strtof	(float)strtod
#endif

namespace VSTGUI {
namespace UIViewCreator {

//-----------------------------------------------------------------------------
bool parseSize (const std::string& str, CPoint& point)
{
	size_t sep = str.find (',', 0);
	if (sep != std::string::npos)
	{
		point.x = strtol (str.c_str (), 0, 10);
		point.y = strtol (str.c_str () + sep+1, 0, 10);
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
bool pointToString (const CPoint& p, std::string& string)
{
	std::stringstream stream;
	stream << p.x;
	stream << ", ";
	stream << p.y;
	string = stream.str ();
	return true;
}

//-----------------------------------------------------------------------------
bool bitmapToString (CBitmap* bitmap, std::string& string, IUIDescription* desc)
{
	UTF8StringPtr bitmapName = desc->lookupBitmapName (bitmap);
	if (bitmapName)
		string = bitmapName;
	else
	{
		const CResourceDescription& res = bitmap->getResourceDescription ();
		if (res.type == CResourceDescription::kStringType)
			string = res.u.name;
		else
		{
			std::stringstream stream;
			stream << res.u.id;
			string = stream.str ();
		}
	}
	return true;
}

//-----------------------------------------------------------------------------
bool colorToString (const CColor& color, std::string& string, IUIDescription* desc)
{
	UTF8StringPtr colorName = desc ? desc->lookupColorName (color) : 0;
	if (colorName)
		string = colorName;
	else
	{
		uint8_t red = color.red;
		uint8_t green = color.green;
		uint8_t blue = color.blue;
		uint8_t alpha = color.alpha;
		char strBuffer[10];
		sprintf (strBuffer, "#%02x%02x%02x%02x", red, green, blue, alpha);
		string = strBuffer;
	}
	return true;
}

//-----------------------------------------------------------------------------
class CViewCreator : public IViewCreator
{
public:
	CViewCreator () { UIViewFactory::registerViewCreator (*this); }
	IdStringPtr getViewName () const { return "CView"; }
	IdStringPtr getBaseViewName () const { return 0; }
	CView* create (const UIAttributes& attributes, IUIDescription* description) const { return new CView (CRect (0, 0, 0, 0)); }
	bool apply (CView* view, const UIAttributes& attributes, IUIDescription* description) const
	{
		const std::string* originAttr = attributes.getAttributeValue ("origin");
		const std::string* sizeAttr = attributes.getAttributeValue ("size");
		const std::string* transparentAttr = attributes.getAttributeValue ("transparent");
		const std::string* mouseEnabledAttr = attributes.getAttributeValue ("mouse-enabled");
		const std::string* bitmapAttr = attributes.getAttributeValue ("bitmap");
		const std::string* disabledBitmapAttr = attributes.getAttributeValue ("disabled-bitmap");
		const std::string* autosizeAttr = attributes.getAttributeValue ("autosize");
		const std::string* tooltipAttr = attributes.getAttributeValue ("tooltip");
		const std::string* customViewAttr = attributes.getAttributeValue ("custom-view-name");
		const std::string* subControllerAttr = attributes.getAttributeValue ("sub-controller");
		
		CPoint p;
		CRect size;
		if (originAttr)
		{
			if (parseSize (*originAttr, p))
			{
				CRect origViewSize = view->getViewSize ();
				size.setTopLeft (p);
				size.setWidth (origViewSize.getWidth ());
				size.setHeight (origViewSize.getHeight ());
				view->setViewSize (size, false);
				view->setMouseableArea (size);
			}
		}
		if (sizeAttr)
		{
			if (parseSize (*sizeAttr, p))
			{
				size = view->getViewSize ();
				size.setSize (p);
				view->setViewSize (size, false);
				view->setMouseableArea (size);
			}
		}
		if (bitmapAttr)
		{
			CBitmap* bitmap = description->getBitmap (bitmapAttr->c_str ());
			view->setBackground (bitmap);
		}
		
		if (disabledBitmapAttr)
		{
			CBitmap* bitmap = description->getBitmap (disabledBitmapAttr->c_str ());
			view->setDisabledBackground (bitmap);
		}
		
		if (transparentAttr)
			view->setTransparency (*transparentAttr == "true");

		if (mouseEnabledAttr)
			view->setMouseEnabled (*mouseEnabledAttr == "true");

		if (autosizeAttr)
		{
			int32_t autosize = kAutosizeNone;
			if (autosizeAttr->find ("left") != std::string::npos)
				autosize |= kAutosizeLeft;
			if (autosizeAttr->find ("top") != std::string::npos)
				autosize |= kAutosizeTop;
			if (autosizeAttr->find ("right") != std::string::npos)
				autosize |= kAutosizeRight;
			if (autosizeAttr->find ("bottom") != std::string::npos)
				autosize |= kAutosizeBottom;
			if (autosizeAttr->find ("row") != std::string::npos)
				autosize |= kAutosizeRow;
			if (autosizeAttr->find ("column") != std::string::npos)
				autosize |= kAutosizeColumn;
			view->setAutosizeFlags (autosize);
		}
		if (tooltipAttr)
		{
			if (tooltipAttr->size () > 0)
				view->setAttribute (kCViewTooltipAttribute, (int32_t)tooltipAttr->size ()+1, tooltipAttr->c_str ());
			else
				view->removeAttribute (kCViewTooltipAttribute);
		}
		if (customViewAttr)
			view->setAttribute ('uicv', (int32_t)customViewAttr->size ()+1, customViewAttr->c_str ());

		if (subControllerAttr)
			view->setAttribute ('uisc', (int32_t)subControllerAttr->size ()+1, subControllerAttr->c_str ());

		return true;
	}
	bool getAttributeNames (std::list<std::string>& attributeNames) const
	{
		attributeNames.push_back ("origin");
		attributeNames.push_back ("size");
		attributeNames.push_back ("transparent");
		attributeNames.push_back ("mouse-enabled");
		attributeNames.push_back ("bitmap");
		attributeNames.push_back ("disabled-bitmap");
		attributeNames.push_back ("autosize");
		attributeNames.push_back ("tooltip");
		attributeNames.push_back ("custom-view-name");
		attributeNames.push_back ("sub-controller");
		return true;
	}
	AttrType getAttributeType (const std::string& attributeName) const
	{
		if (attributeName == "origin") return kPointType;
		else if (attributeName == "size") return kPointType;
		else if (attributeName == "transparent") return kBooleanType;
		else if (attributeName == "mouse-enabled") return kBooleanType;
		else if (attributeName == "bitmap") return kBitmapType;
		else if (attributeName == "disabled-bitmap") return kBitmapType;
		else if (attributeName == "autosize") return kStringType;
		else if (attributeName == "tooltip") return kStringType;
		else if (attributeName == "custom-view-name") return kStringType;
		else if (attributeName == "sub-controller") return kStringType;
		return kUnknownType;
	}
	bool getAttributeValue (CView* view, const std::string& attributeName, std::string& stringValue, IUIDescription* desc) const
	{
		if (attributeName == "origin")
		{
			pointToString (view->getViewSize ().getTopLeft (), stringValue);
			return true;
		}
		else if (attributeName == "size")
		{
			pointToString (view->getViewSize ().getSize (), stringValue);
			return true;
		}
		else if (attributeName == "transparent")
		{
			stringValue = view->getTransparency () ? "true" : "false";
			return true;
		}
		else if (attributeName == "mouse-enabled")
		{
			stringValue = view->getMouseEnabled () ? "true" : "false";
			return true;
		}
		else if (attributeName == "bitmap")
		{
			CBitmap* bitmap = view->getBackground ();
			if (bitmap)
				bitmapToString (bitmap, stringValue, desc);
			else
				stringValue = "";
			return true;
		}
		else if (attributeName == "disabled-bitmap")
		{
			CBitmap* bitmap = view->getDisabledBackground ();
			if (bitmap)
				bitmapToString (bitmap, stringValue, desc);
			else
				stringValue = "";
			return true;
		}
		else if (attributeName == "autosize")
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
		else if (attributeName == "tooltip")
		{
			char* tooltip = 0;
			int32_t tooltipSize = 0;
			if (view->getAttributeSize (kCViewTooltipAttribute, tooltipSize))
			{
				tooltip = (char*)malloc (tooltipSize + 1);
				memset (tooltip, 0, tooltipSize+1);
				if (view->getAttribute (kCViewTooltipAttribute, tooltipSize, tooltip, tooltipSize))
					stringValue = tooltip;
				free (tooltip);
				return true;
			}
		}
		else if (attributeName == "custom-view-name")
		{
			char* customViewName = 0;
			int32_t customViewNameSize = 0;
			if (view->getAttributeSize ('uicv', customViewNameSize))
			{
				customViewName = (char*)malloc (customViewNameSize + 1);
				memset (customViewName, 0, customViewNameSize+1);
				if (view->getAttribute ('uicv', customViewNameSize, customViewName, customViewNameSize))
					stringValue = customViewName;
				free (customViewName);
				return true;
			}
		}
		else if (attributeName == "sub-controller")
		{
			char* subControllerName = 0;
			int32_t subControllerNameSize = 0;
			if (view->getAttributeSize ('uisc', subControllerNameSize))
			{
				subControllerName = (char*)malloc (subControllerNameSize + 1);
				memset (subControllerName, 0, subControllerNameSize+1);
				if (view->getAttribute ('uisc', subControllerNameSize, subControllerName, subControllerNameSize))
					stringValue = subControllerName;
				free (subControllerName);
				return true;
			}
		}
		return false;
	}
};
CViewCreator __gCViewCreator;

//-----------------------------------------------------------------------------
static uint32_t DJBHash (const std::string& str)
{
   uint32_t hash = 5381;
   for (std::size_t i = 0; i < str.length (); i++)
   {
      hash = ((hash << 5) + hash) + str[i];
   }
   return hash;
}

//-----------------------------------------------------------------------------
void rememberAttributeValueString (CView* view, IdStringPtr attrName, const std::string& value)
{
	#if VSTGUI_LIVE_EDITING
	uint32_t hash = DJBHash (attrName);
	view->setAttribute (hash, (int32_t)value.size () + 1, value.c_str ());
	#endif
}

//-----------------------------------------------------------------------------
bool getRememberedAttributeValueString (CView* view, IdStringPtr attrName, std::string& value)
{
	bool result = false;
	#if VSTGUI_LIVE_EDITING
	uint32_t hash = DJBHash (attrName);
	int32_t attrSize = 0;
	if (view->getAttributeSize (hash, attrSize))
	{
		char* temp = new char[attrSize];
		if (view->getAttribute (hash, attrSize, temp, attrSize))
		{
			value = temp;
			result = true;
		}
		delete [] temp;
	}
	#endif
	return result;
}

//-----------------------------------------------------------------------------
class CViewContainerCreator : public IViewCreator
{
public:
	CViewContainerCreator () { UIViewFactory::registerViewCreator (*this); }
	IdStringPtr getViewName () const { return "CViewContainer"; }
	IdStringPtr getBaseViewName () const { return "CView"; }
	CView* create (const UIAttributes& attributes, IUIDescription* description) const { return new CViewContainer (CRect (0, 0, 100, 100)); }
	bool apply (CView* view, const UIAttributes& attributes, IUIDescription* description) const
	{
		CViewContainer* viewContainer = dynamic_cast<CViewContainer*> (view);
		if (viewContainer == 0)
			return false;
		const std::string* attr = attributes.getAttributeValue ("background-color");
		if (attr)
		{
			CColor backColor;
			if (description->getColor (attr->c_str (), backColor))
			{
				rememberAttributeValueString (view, "background-color", *attr);
				viewContainer->setBackgroundColor (backColor);
			}
		}
		attr = attributes.getAttributeValue ("background-color-draw-style");
		if (attr)
		{
			CDrawStyle drawStyle = kDrawFilledAndStroked;
			if (*attr == "stroked")
			{
				drawStyle = kDrawStroked;
			}
			else if (*attr == "filled")
			{
				drawStyle = kDrawFilled;
			}
			viewContainer->setBackgroundColorDrawStyle (drawStyle);
		}
		return true;
	}
	bool getAttributeNames (std::list<std::string>& attributeNames) const
	{
		attributeNames.push_back ("background-color");
		attributeNames.push_back ("background-color-draw-style");
		return true;
	}
	AttrType getAttributeType (const std::string& attributeName) const
	{
		if (attributeName == "background-color") return kColorType;
		if (attributeName == "background-color-draw-style") return kListType;
		return kUnknownType;
	}
	bool getAttributeValue (CView* view, const std::string& attributeName, std::string& stringValue, IUIDescription* desc) const
	{
		CViewContainer* vc = dynamic_cast<CViewContainer*> (view);
		if (vc == 0)
			return false;
		if (attributeName == "background-color")
		{
			if (!getRememberedAttributeValueString (view, "background-color", stringValue))
				colorToString (vc->getBackgroundColor (), stringValue, desc);
			return true;
		}
		if (attributeName == "background-color-draw-style")
		{
			switch (vc->getBackgroundColorDrawStyle ())
			{
				case kDrawStroked: stringValue = "stroked"; break;
				case kDrawFilledAndStroked: stringValue = "filled and stroked"; break;
				case kDrawFilled: stringValue = "filled"; break;
			}
			return true;
		}
		return false;
	}

	bool getPossibleListValues (const std::string& attributeName, std::list<const std::string*>& values) const
	{
		if (attributeName == "background-color-draw-style")
		{
			static std::string kStroked = "stroked";
			static std::string kFilledAndStroked = "filled and stroked";
			static std::string kFilled = "filled";
			values.push_back (&kStroked);
			values.push_back (&kFilledAndStroked);
			values.push_back (&kFilled);
			return true;
		}
		return false;
	}

};
CViewContainerCreator __CViewContainerCreator;

//-----------------------------------------------------------------------------
class CLayeredViewContainerCreator : public IViewCreator
{
public:
	CLayeredViewContainerCreator () { UIViewFactory::registerViewCreator (*this); }
	IdStringPtr getViewName () const { return "CLayeredViewContainer"; }
	IdStringPtr getBaseViewName () const { return "CViewContainer"; }
	CView* create (const UIAttributes& attributes, IUIDescription* description) const { return new CLayeredViewContainer (CRect (0, 0, 100, 100)); }
	bool apply (CView* view, const UIAttributes& attributes, IUIDescription* description) const
	{
		return true;
	}
	bool getAttributeNames (std::list<std::string>& attributeNames) const
	{
		return true;
	}
	AttrType getAttributeType (const std::string& attributeName) const
	{
		return kUnknownType;
	}
	bool getAttributeValue (CView* view, const std::string& attributeName, std::string& stringValue, IUIDescription* desc) const
	{
		return false;
	}
};
CLayeredViewContainerCreator __CLayeredViewContainerCreator;

//-----------------------------------------------------------------------------
class CRowColumnViewCreator : public IViewCreator
{
public:
	CRowColumnViewCreator () { UIViewFactory::registerViewCreator (*this); }
	IdStringPtr getViewName () const { return "CRowColumnView"; }
	IdStringPtr getBaseViewName () const { return "CViewContainer"; }
	CView* create (const UIAttributes& attributes, IUIDescription* description) const { return new CRowColumnView (CRect (0, 0, 100, 100)); }
	bool apply (CView* view, const UIAttributes& attributes, IUIDescription* description) const
	{
		CRowColumnView* rcv = dynamic_cast<CRowColumnView*> (view);
		if (rcv == 0)
			return false;
		const std::string* attr = attributes.getAttributeValue ("row-style");
		if (attr)
			rcv->setStyle (*attr == "true" ? CRowColumnView::kRowStyle : CRowColumnView::kColumnStyle);
		attr = attributes.getAttributeValue ("spacing");
		if (attr)
		{
			CCoord spacing = strtof (attr->c_str (), 0);
			rcv->setSpacing (spacing);
		}
		CRect margin;
		if (attributes.getRectAttribute ("margin", margin))
			rcv->setMargin (margin);
		attr = attributes.getAttributeValue ("animate-view-resizing");
		if (attr)
			rcv->setAnimateViewResizing (*attr == "true" ? true : false);
		attr = attributes.getAttributeValue ("equal-size-layout");
		if (attr)
		{
			if (*attr == "stretch")
				rcv->setLayoutStyle (CRowColumnView::kStretchEqualy);
			else if (*attr == "center")
				rcv->setLayoutStyle (CRowColumnView::kCenterEqualy);
			else if (*attr == "right-bottom")
				rcv->setLayoutStyle (CRowColumnView::kRightBottomEqualy);
			else
				rcv->setLayoutStyle (CRowColumnView::kLeftTopEqualy);
		}
		attr = attributes.getAttributeValue ("view-resize-animation-time");
		if (attr)
		{
			uint32_t time = (uint32_t)strtol (attr->c_str(), 0, 10);
			rcv->setViewResizeAnimationTime (time);
		}
		return true;
	}
	bool getAttributeNames (std::list<std::string>& attributeNames) const
	{
		attributeNames.push_back ("row-style");
		attributeNames.push_back ("spacing");
		attributeNames.push_back ("margin");
		attributeNames.push_back ("equal-size-layout");
		attributeNames.push_back ("animate-view-resizing");
		attributeNames.push_back ("view-resize-animation-time");
		return true;
	}
	AttrType getAttributeType (const std::string& attributeName) const
	{
		if (attributeName == "row-style") return kBooleanType;
		if (attributeName == "spacing") return kIntegerType;
		if (attributeName == "margin") return kRectType;
		if (attributeName == "equal-size-layout") return kListType;
		if (attributeName == "animate-view-resizing") return kBooleanType;
		if (attributeName == "view-resize-animation-time") return kIntegerType;
		return kUnknownType;
	}
	bool getAttributeValue (CView* view, const std::string& attributeName, std::string& stringValue, IUIDescription* desc) const
	{
		CRowColumnView* rcv = dynamic_cast<CRowColumnView*> (view);
		if (rcv == 0)
			return false;
		if (attributeName == "row-style")
		{
			stringValue = rcv->getStyle () == CRowColumnView::kRowStyle ? "true" : "false";
			return true;
		}
		if (attributeName == "animate-view-resizing")
		{
			stringValue = rcv->isAnimateViewResizing () ? "true" : "false";
			return true;
		}
		if (attributeName == "spacing")
		{
			std::stringstream str;
			str << (int32_t)rcv->getSpacing ();
			stringValue = str.str ();
			return true;
		}
		if (attributeName == "view-resize-animation-time")
		{
			std::stringstream str;
			str << rcv->getViewResizeAnimationTime ();
			stringValue = str.str ();
			return true;
		}
		if (attributeName == "margin")
		{
			const CRect& margin = rcv->getMargin ();
			std::stringstream str;
			str << (int32_t)margin.left;
			str << ",";
			str << (int32_t)margin.top;
			str << ",";
			str << (int32_t)margin.right;
			str << ",";
			str << (int32_t)margin.bottom;
			stringValue = str.str ();
			return true;
		}
		if (attributeName == "equal-size-layout")
		{
			switch (rcv->getLayoutStyle ())
			{
				case CRowColumnView::kLeftTopEqualy: stringValue = "left-top"; break;
				case CRowColumnView::kStretchEqualy: stringValue = "stretch"; break;
				case CRowColumnView::kCenterEqualy: stringValue = "center"; break;
				case CRowColumnView::kRightBottomEqualy: stringValue = "right-bottom"; break;
			}
			return true;
		}
		return false;
	}
	bool getPossibleListValues (const std::string& attributeName, std::list<const std::string*>& values) const
	{
		if (attributeName == "equal-size-layout")
		{
			static std::string kLeftTop = "left-top";
			static std::string kStretch = "stretch";
			static std::string kCenter = "center";
			static std::string kRightBottom = "right-bottom";
	
			values.push_back (&kLeftTop);
			values.push_back (&kStretch);
			values.push_back (&kCenter);
			values.push_back (&kRightBottom);
			return true;
		}
		return false;
	}

};
CRowColumnViewCreator __CRowColumnViewCreator;

//-----------------------------------------------------------------------------
class CScrollViewCreator : public IViewCreator
{
public:
	CScrollViewCreator () { UIViewFactory::registerViewCreator (*this); }
	IdStringPtr getViewName () const { return "CScrollView"; }
	IdStringPtr getBaseViewName () const { return "CViewContainer"; }
	CView* create (const UIAttributes& attributes, IUIDescription* description) const { return new CScrollView (CRect (0, 0, 100, 100), CRect (0, 0, 200, 200), CScrollView::kHorizontalScrollbar|CScrollView::kVerticalScrollbar); }
	bool apply (CView* view, const UIAttributes& attributes, IUIDescription* description) const
	{
		CScrollView* scrollView = dynamic_cast<CScrollView*> (view);
		if (scrollView == 0)
			return false;
		const std::string* attr = attributes.getAttributeValue ("container-size");
		if (attr)
		{
			CPoint p;
			if (parseSize (*attr, p))
			{
				CRect r;
				r.setSize (p);
				scrollView->setContainerSize (r);
			}
		}
		int32_t style = scrollView->getStyle ();
		attr = attributes.getAttributeValue ("horizontal-scrollbar");
		if (attr)
		{
			if (*attr == "true")
				style |= CScrollView::kHorizontalScrollbar;
			else
				style &= ~CScrollView::kHorizontalScrollbar;
		}
		attr = attributes.getAttributeValue ("vertical-scrollbar");
		if (attr)
		{
			if (*attr == "true")
				style |= CScrollView::kVerticalScrollbar;
			else
				style &= ~CScrollView::kVerticalScrollbar;
		}
		attr = attributes.getAttributeValue ("auto-drag-scrolling");
		if (attr)
		{
			if (*attr == "true")
				style |= CScrollView::kAutoDragScrolling;
			else
				style &= ~CScrollView::kAutoDragScrolling;
		}
		attr = attributes.getAttributeValue ("bordered");
		if (attr)
		{
			if (*attr == "true")
				style &= ~CScrollView::kDontDrawFrame;
			else
				style |= CScrollView::kDontDrawFrame;
		}
		attr = attributes.getAttributeValue ("overlay-scrollbars");
		if (attr)
		{
			if (*attr == "true")
				style |= CScrollView::kOverlayScrollbars;
			else
				style &= ~CScrollView::kOverlayScrollbars;
		}
		attr = attributes.getAttributeValue ("follow-focus-view");
		if (attr)
		{
			if (*attr == "true")
				style |= CScrollView::kFollowFocusView;
			else
				style &= ~CScrollView::kFollowFocusView;
		}
		attr = attributes.getAttributeValue ("auto-hide-scrollbars");
		if (attr)
		{
			if (*attr == "true")
				style |= CScrollView::kAutoHideScrollbars;
			else
				style &= ~CScrollView::kAutoHideScrollbars;
		}
		scrollView->setStyle (style);
		CColor color;
		CScrollbar* vscrollbar = scrollView->getVerticalScrollbar ();
		CScrollbar* hscrollbar = scrollView->getHorizontalScrollbar ();
		attr = attributes.getAttributeValue ("scrollbar-background-color");
		if (attr)
		{
			if (description->getColor (attr->c_str (), color))
			{
				rememberAttributeValueString (view, "scrollbar-background-color", *attr);
				if (vscrollbar) vscrollbar->setBackgroundColor (color);
				if (hscrollbar) hscrollbar->setBackgroundColor (color);
			}
		}
		attr = attributes.getAttributeValue ("scrollbar-frame-color");
		if (attr)
		{
			if (description->getColor (attr->c_str (), color))
			{
				rememberAttributeValueString (view, "scrollbar-frame-color", *attr);
				if (vscrollbar) vscrollbar->setFrameColor (color);
				if (hscrollbar) hscrollbar->setFrameColor (color);
			}
		}
		attr = attributes.getAttributeValue ("scrollbar-scroller-color");
		if (attr)
		{
			if (description->getColor (attr->c_str (), color))
			{
				rememberAttributeValueString (view, "scrollbar-scroller-color", *attr);
				if (vscrollbar) vscrollbar->setScrollerColor (color);
				if (hscrollbar) hscrollbar->setScrollerColor (color);
			}
		}
		attr = attributes.getAttributeValue ("scrollbar-width");
		if (attr)
		{
			CCoord width = strtof (attr->c_str (), 0);
			scrollView->setScrollbarWidth (width);
		}
		return true;
	}
	bool getAttributeNames (std::list<std::string>& attributeNames) const
	{
		attributeNames.push_back ("container-size");
		attributeNames.push_back ("scrollbar-background-color");
		attributeNames.push_back ("scrollbar-frame-color");
		attributeNames.push_back ("scrollbar-scroller-color");
		attributeNames.push_back ("horizontal-scrollbar");
		attributeNames.push_back ("vertical-scrollbar");
		attributeNames.push_back ("auto-hide-scrollbars");
		attributeNames.push_back ("auto-drag-scrolling");
		attributeNames.push_back ("overlay-scrollbars");
		attributeNames.push_back ("scrollbar-width");
		attributeNames.push_back ("bordered");
		attributeNames.push_back ("follow-focus-view");
		return true;
	}
	AttrType getAttributeType (const std::string& attributeName) const
	{
		if (attributeName == "container-size") return kPointType;
		if (attributeName == "scrollbar-background-color") return kColorType;
		if (attributeName == "scrollbar-frame-color") return kColorType;
		if (attributeName == "scrollbar-scroller-color") return kColorType;
		if (attributeName == "horizontal-scrollbar") return kBooleanType;
		if (attributeName == "vertical-scrollbar") return kBooleanType;
		if (attributeName == "auto-hide-scrollbars") return kBooleanType;
		if (attributeName == "auto-drag-scrolling") return kBooleanType;
		if (attributeName == "overlay-scrollbars") return kBooleanType;
		if (attributeName == "scrollbar-width") return kIntegerType;
		if (attributeName == "bordered") return kBooleanType;
		if (attributeName == "follow-focus-view") return kBooleanType;
		return kUnknownType;
	}
	bool getAttributeValue (CView* view, const std::string& attributeName, std::string& stringValue, IUIDescription* desc) const
	{
		CScrollView* sc = dynamic_cast<CScrollView*> (view);
		if (sc == 0)
			return false;
		if (attributeName == "container-size")
		{
			pointToString (sc->getContainerSize ().getSize (), stringValue);
			return true;
		}
		if (attributeName == "scrollbar-width")
		{
			std::stringstream str;
			str << sc->getScrollbarWidth ();
			stringValue = str.str ();
			return true;
		}
		CScrollbar* scrollbar = sc->getVerticalScrollbar ();
		if (!scrollbar)
			scrollbar = sc->getHorizontalScrollbar ();
		if (scrollbar)
		{
			if (attributeName == "scrollbar-background-color")
			{
				if (!getRememberedAttributeValueString (view, "scrollbar-background-color", stringValue))
					colorToString (scrollbar->getBackgroundColor (), stringValue, desc);
				return true;
			}
			if (attributeName == "scrollbar-frame-color")
			{
				if (!getRememberedAttributeValueString (view, "scrollbar-frame-color", stringValue))
					colorToString (scrollbar->getFrameColor (), stringValue, desc);
				return true;
			}
			if (attributeName == "scrollbar-scroller-color")
			{
				if (!getRememberedAttributeValueString (view, "scrollbar-scroller-color", stringValue))
					colorToString (scrollbar->getScrollerColor (), stringValue, desc);
				return true;
			}
		}
		if (attributeName == "horizontal-scrollbar")
		{
			stringValue = sc->getStyle () & CScrollView::kHorizontalScrollbar ? "true" : "false";
			return true;
		}
		if (attributeName == "vertical-scrollbar")
		{
			stringValue = sc->getStyle () & CScrollView::kVerticalScrollbar ? "true" : "false";
			return true;
		}
		if (attributeName == "auto-hide-scrollbars")
		{
			stringValue = sc->getStyle () & CScrollView::kAutoHideScrollbars ? "true" : "false";
			return true;
		}
		if (attributeName == "auto-drag-scrolling")
		{
			stringValue = sc->getStyle () & CScrollView::kAutoDragScrolling ? "true" : "false";
			return true;
		}
		if (attributeName == "bordered")
		{
			stringValue = sc->getStyle () & CScrollView::kDontDrawFrame ? "false" : "true";
			return true;
		}
		if (attributeName == "overlay-scrollbars")
		{
			stringValue = sc->getStyle () & CScrollView::kOverlayScrollbars ? "true" : "false";
			return true;
		}
		if (attributeName == "follow-focus-view")
		{
			stringValue = sc->getStyle () & CScrollView::kFollowFocusView ? "true" : "false";
			return true;
		}
		return false;
	}

};
CScrollViewCreator __CScrollViewCreator;

//-----------------------------------------------------------------------------
class CControlCreator : public IViewCreator
{
protected:
	class DummyControl : public CControl
	{
	public:
		DummyControl () : CControl (CRect (0, 0, 0, 0)) {}
		void draw (CDrawContext* pContext) { CView::draw (pContext); }
		
		CLASS_METHODS(DummyControl, CControl)
	};
public:
	CControlCreator () { UIViewFactory::registerViewCreator (*this); }
	IdStringPtr getViewName () const { return "CControl"; }
	IdStringPtr getBaseViewName () const { return "CView"; }
	CView* create (const UIAttributes& attributes, IUIDescription* description) const { return new DummyControl (); }
	bool apply (CView* view, const UIAttributes& attributes, IUIDescription* description) const
	{
		CControl* control = dynamic_cast<CControl*> (view);
		if (control == 0)
			return false;
		const std::string* controlTagAttr = attributes.getAttributeValue ("control-tag");
		const std::string* defaultValueAttr = attributes.getAttributeValue ("default-value");
		const std::string* minValueAttr = attributes.getAttributeValue ("min-value");
		const std::string* maxValueAttr = attributes.getAttributeValue ("max-value");
		const std::string* wheelIncValueAttr = attributes.getAttributeValue ("wheel-inc-value");
		const std::string* backOffsetAttr = attributes.getAttributeValue ("background-offset");
		
		float value = 0.f;
		if (defaultValueAttr)
		{
			value = strtof (defaultValueAttr->c_str (), 0);
			control->setDefaultValue (value);
		}
		if (minValueAttr)
		{
			value = strtof (minValueAttr->c_str (), 0);
			control->setMin (value);
		}
		if (maxValueAttr)
		{
			value = strtof (maxValueAttr->c_str (), 0);
			control->setMax (value);
		}
		if (wheelIncValueAttr)
		{
			value = strtof (wheelIncValueAttr->c_str (), 0);
			control->setWheelInc (value);
		}
		if (backOffsetAttr)
		{
			CPoint p;
			if (parseSize (*backOffsetAttr, p))
				control->setBackOffset (p);
		}
		if (controlTagAttr)
		{
			rememberAttributeValueString (view, "control-tag", *controlTagAttr);
			if (controlTagAttr->length () == 0)
			{
				control->setTag (-1);
				control->setListener (0);
			}
			else
			{
				int32_t tag = description->getTagForName (controlTagAttr->c_str ());
				if (tag != -1)
				{
					control->setTag (tag);
					control->setListener (description->getControlListener (controlTagAttr->c_str ()));
				}
				else
				{
					char* endPtr = 0;
					tag = (int32_t)strtol (controlTagAttr->c_str (), &endPtr, 10);
					if (endPtr != controlTagAttr->c_str ())
					{
						control->setTag (tag);
						control->setListener (description->getControlListener (controlTagAttr->c_str ()));
					}
				}
			}
		}
		
		return true;
	}
	bool getAttributeNames (std::list<std::string>& attributeNames) const
	{
		attributeNames.push_back ("control-tag");
		attributeNames.push_back ("default-value");
		attributeNames.push_back ("min-value");
		attributeNames.push_back ("max-value");
		attributeNames.push_back ("wheel-inc-value");
		attributeNames.push_back ("background-offset");
		return true;
	}
	AttrType getAttributeType (const std::string& attributeName) const
	{
		if (attributeName == "control-tag") return kTagType;
		else if (attributeName == "default-value") return kFloatType;
		else if (attributeName == "min-value") return kFloatType;
		else if (attributeName == "max-value") return kFloatType;
		else if (attributeName == "wheel-inc-value") return kFloatType;
		else if (attributeName == "background-offset") return kPointType;
		return kUnknownType;
	}
	bool getAttributeValue (CView* view, const std::string& attributeName, std::string& stringValue, IUIDescription* desc) const
	{
		CControl* control = dynamic_cast<CControl*> (view);
		if (control == 0)
			return false;
		std::stringstream stream;
		if (attributeName == "control-tag")
		{
			if (control->getTag () != -1)
			{
				if (getRememberedAttributeValueString (view, "control-tag", stringValue))
					return true;
				UTF8StringPtr controlTag = desc->lookupControlTagName (control->getTag ());
				if (controlTag)
				{
					stringValue = controlTag;
					return true;
				}
			}
		}
		else if (attributeName == "default-value")
		{
			stream << control->getDefaultValue ();
			stringValue = stream.str ();
			return true;
		}
		else if (attributeName == "min-value")
		{
			stream << control->getMin ();
			stringValue = stream.str ();
			return true;
		}
		else if (attributeName == "max-value")
		{
			stream << control->getMax ();
			stringValue = stream.str ();
			return true;
		}
		else if (attributeName == "wheel-inc-value")
		{
			stream << control->getWheelInc ();
			stringValue = stream.str ();
			return true;
		}
		else if (attributeName == "background-offset")
		{
			pointToString (control->getBackOffset (), stringValue);
			return true;
		}
		return false;
	}

};
CControlCreator __gCControlCreator;

//-----------------------------------------------------------------------------
class COnOffButtonCreator : public IViewCreator
{
public:
	COnOffButtonCreator () { UIViewFactory::registerViewCreator (*this); }
	IdStringPtr getViewName () const { return "COnOffButton"; }
	IdStringPtr getBaseViewName () const { return "CControl"; }
	CView* create (const UIAttributes& attributes, IUIDescription* description) const { return new COnOffButton (CRect (0, 0, 0, 0), 0, -1, 0); }
	bool apply (CView* view, const UIAttributes& attributes, IUIDescription* description) const
	{
		return true;
	}
	bool getAttributeNames (std::list<std::string>& attributeNames) const
	{
		return true;
	}
	AttrType getAttributeType (const std::string& attributeName) const
	{
		return kUnknownType;
	}
	bool getAttributeValue (CView* view, const std::string& attributeName, std::string& stringValue, IUIDescription* desc) const
	{
		return false;
	}

};
COnOffButtonCreator __gCOnOffButtonCreator;

//-----------------------------------------------------------------------------
class CCheckBoxCreator : public IViewCreator
{
public:
	CCheckBoxCreator () { UIViewFactory::registerViewCreator (*this); }
	IdStringPtr getViewName () const { return "CCheckBox"; }
	IdStringPtr getBaseViewName () const { return "CControl"; }
	CView* create (const UIAttributes& attributes, IUIDescription* description) const { return new CCheckBox (CRect (0, 0, 100, 20), 0, -1, "Title"); }
	bool apply (CView* view, const UIAttributes& attributes, IUIDescription* description) const
	{
		CCheckBox* checkbox = dynamic_cast<CCheckBox*> (view);
		if (!checkbox)
			return false;
		
		const std::string* attr = attributes.getAttributeValue ("title");
		if (attr)
			checkbox->setTitle (attr->c_str ());

		attr = attributes.getAttributeValue ("font");
		if (attr)
		{
			CFontRef font = description->getFont (attr->c_str ());
			if (font)
			{
				rememberAttributeValueString (view, "font", *attr);
				checkbox->setFont (font);
			}
		}

		CColor color;
		attr = attributes.getAttributeValue ("font-color");
		if (attr)
		{
			if (description->getColor (attr->c_str (), color))
			{
				rememberAttributeValueString (view, "font-color", *attr);
				checkbox->setFontColor (color);
			}
		}

		attr = attributes.getAttributeValue ("boxframe-color");
		if (attr)
		{
			if (description->getColor (attr->c_str (), color))
			{
				rememberAttributeValueString (view, "boxframe-color", *attr);
				checkbox->setBoxFrameColor (color);
			}
		}

		attr = attributes.getAttributeValue ("boxfill-color");
		if (attr)
		{
			if (description->getColor (attr->c_str (), color))
			{
				rememberAttributeValueString (view, "boxfill-color", *attr);
				checkbox->setBoxFillColor (color);
			}
		}

		attr = attributes.getAttributeValue ("checkmark-color");
		if (attr)
		{
			if (description->getColor (attr->c_str (), color))
			{
				rememberAttributeValueString (view, "checkmark-color", *attr);
				checkbox->setCheckMarkColor (color);
			}
		}
		int32_t style = checkbox->getStyle ();
		attr = attributes.getAttributeValue ("draw-crossbox");
		if (attr)
		{
			if (*attr == "true")
				style |= CCheckBox::kDrawCrossBox;
			else
				style &= ~CCheckBox::kDrawCrossBox;
		}
		attr = attributes.getAttributeValue ("autosize-to-fit");
		if (attr)
		{
			if (*attr == "true")
				style |= CCheckBox::kAutoSizeToFit;
			else
				style &= ~CCheckBox::kAutoSizeToFit;
		}
		checkbox->setStyle (style);
		
		return true;
	}

	bool getAttributeNames (std::list<std::string>& attributeNames) const
	{
		attributeNames.push_back ("title");
		attributeNames.push_back ("font");
		attributeNames.push_back ("font-color");
		attributeNames.push_back ("boxframe-color");
		attributeNames.push_back ("boxfill-color");
		attributeNames.push_back ("checkmark-color");
		attributeNames.push_back ("autosize-to-fit");
		attributeNames.push_back ("draw-crossbox");
		return true;
	}
	AttrType getAttributeType (const std::string& attributeName) const
	{
		if (attributeName == "title") return kStringType;
		else if (attributeName == "font") return kFontType;
		else if (attributeName == "font-color") return kColorType;
		else if (attributeName == "boxframe-color") return kColorType;
		else if (attributeName == "boxfill-color") return kColorType;
		else if (attributeName == "checkmark-color") return kColorType;
		else if (attributeName == "autosize-to-fit") return kBooleanType;
		else if (attributeName == "draw-crossbox") return kBooleanType;
		return kUnknownType;
	}
	bool getAttributeValue (CView* view, const std::string& attributeName, std::string& stringValue, IUIDescription* desc) const
	{
		CCheckBox* checkbox = dynamic_cast<CCheckBox*> (view);
		if (!checkbox)
			return false;
		
		if (attributeName == "title")
		{
			stringValue = checkbox->getTitle () ? checkbox->getTitle () : "";
			return true;
		}
		else if (attributeName == "font")
		{
			if (getRememberedAttributeValueString (view, "font", stringValue))
				return true;
			UTF8StringPtr fontName = desc->lookupFontName (checkbox->getFont ());
			if (fontName)
			{
				stringValue = fontName;
				return true;
			}
			return false;
		}
		else if (attributeName == "font-color")
		{
			if (!getRememberedAttributeValueString (view, "font-color", stringValue))
				colorToString (checkbox->getFontColor (), stringValue, desc);
			return true;
		}
		else if (attributeName == "boxframe-color")
		{
			if (!getRememberedAttributeValueString (view, "boxframe-color", stringValue))
				colorToString (checkbox->getBoxFrameColor (), stringValue, desc);
			return true;
		}
		else if (attributeName == "boxfill-color")
		{
			if (!getRememberedAttributeValueString (view, "boxfill-color", stringValue))
				colorToString (checkbox->getBoxFillColor (), stringValue, desc);
			return true;
		}
		else if (attributeName == "checkmark-color")
		{
			if (!getRememberedAttributeValueString (view, "checkmark-color", stringValue))
				colorToString (checkbox->getCheckMarkColor (), stringValue, desc);
			return true;
		}
		else if (attributeName == "autosize-to-fit")
		{
			if (checkbox->getStyle () & CCheckBox::kAutoSizeToFit)
				stringValue = "true";
			else
				stringValue = "false";
			return true;
		}
		else if (attributeName == "draw-crossbox")
		{
			if (checkbox->getStyle () & CCheckBox::kDrawCrossBox)
				stringValue = "true";
			else
				stringValue = "false";
			return true;
		}
		return false;
	}

};
CCheckBoxCreator __gCCheckBoxCreator;

//-----------------------------------------------------------------------------
class CParamDisplayCreator : public IViewCreator
{
public:
	CParamDisplayCreator () { UIViewFactory::registerViewCreator (*this); }
	IdStringPtr getViewName () const { return "CParamDisplay"; }
	IdStringPtr getBaseViewName () const { return "CControl"; }
	CView* create (const UIAttributes& attributes, IUIDescription* description) const { return new CParamDisplay (CRect (0, 0, 0, 0)); }
	bool apply (CView* view, const UIAttributes& attributes, IUIDescription* description) const
	{
		CParamDisplay* display = dynamic_cast<CParamDisplay*> (view);
		if (!display)
			return false;
		
		const std::string* fontAttr = attributes.getAttributeValue ("font");
		const std::string* fontColorAttr = attributes.getAttributeValue ("font-color");
		const std::string* backColorAttr = attributes.getAttributeValue ("back-color");
		const std::string* frameColorAttr = attributes.getAttributeValue ("frame-color");
		const std::string* shadowColorAttr = attributes.getAttributeValue ("shadow-color");
		const std::string* antialiasAttr = attributes.getAttributeValue ("font-antialias");
		const std::string* style3DInAttr = attributes.getAttributeValue ("style-3D-in");
		const std::string* style3DOutAttr = attributes.getAttributeValue ("style-3D-out");
		const std::string* styleNoFrameAttr = attributes.getAttributeValue ("style-no-frame");
		const std::string* styleNoTextAttr = attributes.getAttributeValue ("style-no-text");
		const std::string* styleNoDrawAttr = attributes.getAttributeValue ("style-no-draw");
		const std::string* styleShadowTextAttr = attributes.getAttributeValue ("style-shadow-text");
		const std::string* styleRoundRectAttr = attributes.getAttributeValue ("style-round-rect");
		const std::string* textAlignmentAttr = attributes.getAttributeValue ("text-alignment");
		const std::string* textInsetAttr = attributes.getAttributeValue ("text-inset");
		const std::string* roundRectRadiusAttr = attributes.getAttributeValue ("round-rect-radius");
		const std::string* frameWidthAttr = attributes.getAttributeValue ("frame-width");
		const std::string* precisionAttr = attributes.getAttributeValue ("value-precision");

		CColor color;
		if (fontAttr)
		{
			CFontRef font = description->getFont (fontAttr->c_str ());
			if (font)
			{
				rememberAttributeValueString (view, "font", *fontAttr);
				display->setFont (font);
			}
		}
		if (fontColorAttr)
		{
			if (description->getColor (fontColorAttr->c_str (), color))
			{
				rememberAttributeValueString (view, "font-color", *fontColorAttr);
				display->setFontColor (color);
			}
		}
		if (backColorAttr)
		{
			if (description->getColor (backColorAttr->c_str (), color))
			{
				rememberAttributeValueString (view, "back-color", *backColorAttr);
				display->setBackColor (color);
			}
		}
		if (frameColorAttr)
		{
			if (description->getColor (frameColorAttr->c_str (), color))
			{
				rememberAttributeValueString (view, "frame-color", *frameColorAttr);
				display->setFrameColor (color);
			}
		}
		if (shadowColorAttr)
		{
			if (description->getColor (shadowColorAttr->c_str (), color))
			{
				rememberAttributeValueString (view, "shadow-color", *shadowColorAttr);
				display->setShadowColor (color);
			}
		}
		if (textInsetAttr)
		{
			CPoint p;
			if (parseSize (*textInsetAttr, p))
				display->setTextInset (p);
		}
		if (antialiasAttr)
			display->setAntialias (*antialiasAttr == "true");
		if (textAlignmentAttr)
		{
			CHoriTxtAlign align = kCenterText;
			if (*textAlignmentAttr == "left")
				align = kLeftText;
			else if (*textAlignmentAttr == "right")
				align = kRightText;
			display->setHoriAlign (align);
		}
		if (roundRectRadiusAttr)
		{
			CCoord radius = strtof (roundRectRadiusAttr->c_str (), 0);
			display->setRoundRectRadius (radius);
		}
		if (frameWidthAttr)
		{
			CCoord width = strtof (frameWidthAttr->c_str (), 0);
			display->setFrameWidth (width);
		}
		int32_t style = display->getStyle ();
		if (style3DInAttr)
		{
			if (*style3DInAttr == "true")
				style |= k3DIn;
			else
				style &= ~k3DIn;
		}
		if (style3DOutAttr)
		{
			if (*style3DOutAttr == "true")
				style |= k3DOut;
			else
				style &= ~k3DOut;
		}
		if (styleNoFrameAttr)
		{
			if (*styleNoFrameAttr == "true")
				style |= kNoFrame;
			else
				style &= ~kNoFrame;
		}
		if (styleNoTextAttr)
		{
			if (*styleNoTextAttr == "true")
				style |= kNoTextStyle;
			else
				style &= ~kNoTextStyle;
		}
		if (styleNoDrawAttr)
		{
			if (*styleNoDrawAttr == "true")
				style |= kNoDrawStyle;
			else
				style &= ~kNoDrawStyle;
		}
		if (styleShadowTextAttr)
		{
			if (*styleShadowTextAttr == "true")
				style |= kShadowText;
			else
				style &= ~kShadowText;
		}
		if (styleRoundRectAttr)
		{
			if (*styleRoundRectAttr == "true")
				style |= kRoundRectStyle;
			else
				style &= ~kRoundRectStyle;
		}
		display->setStyle (style);
		if (precisionAttr)
		{
			uint8_t precision = (uint8_t)strtol (precisionAttr->c_str (), 0, 10);
			display->setPrecision (precision);
		}
		return true;
	}
	bool getAttributeNames (std::list<std::string>& attributeNames) const
	{
		attributeNames.push_back ("font");
		attributeNames.push_back ("font-color");
		attributeNames.push_back ("back-color");
		attributeNames.push_back ("frame-color");
		attributeNames.push_back ("shadow-color");
		attributeNames.push_back ("round-rect-radius");
		attributeNames.push_back ("frame-width");
		attributeNames.push_back ("text-alignment");
		attributeNames.push_back ("text-inset");
		attributeNames.push_back ("value-precision");
		attributeNames.push_back ("font-antialias");
		attributeNames.push_back ("style-3D-in");
		attributeNames.push_back ("style-3D-out");
		attributeNames.push_back ("style-no-frame");
		attributeNames.push_back ("style-no-text");
		attributeNames.push_back ("style-no-draw");
		attributeNames.push_back ("style-shadow-text");
		attributeNames.push_back ("style-round-rect");
		return true;
	}
	AttrType getAttributeType (const std::string& attributeName) const
	{
		if (attributeName == "font") return kFontType;
		else if (attributeName == "font-color") return kColorType;
		else if (attributeName == "back-color") return kColorType;
		else if (attributeName == "frame-color") return kColorType;
		else if (attributeName == "shadow-color") return kColorType;
		else if (attributeName == "font-antialias") return kBooleanType;
		else if (attributeName == "style-3D-in")return kBooleanType;
		else if (attributeName == "style-3D-out") return kBooleanType;
		else if (attributeName == "style-no-frame") return kBooleanType;
		else if (attributeName == "style-no-text") return kBooleanType;
		else if (attributeName == "style-no-draw") return kBooleanType;
		else if (attributeName == "style-shadow-text") return kBooleanType;
		else if (attributeName == "style-round-rect") return kBooleanType;
		else if (attributeName == "round-rect-radius") return kFloatType;
		else if (attributeName == "frame-width") return kFloatType;
		else if (attributeName == "text-alignment") return kStringType;
		else if (attributeName == "text-inset") return kPointType;
		else if (attributeName == "value-precision") return kIntegerType;
		return kUnknownType;
	}
	bool getAttributeValue (CView* view, const std::string& attributeName, std::string& stringValue, IUIDescription* desc) const
	{
		CParamDisplay* pd = dynamic_cast<CParamDisplay*> (view);
		if (pd == 0)
			return false;
		if (attributeName == "font")
		{
			if (getRememberedAttributeValueString (view, "font", stringValue))
				return true;
			UTF8StringPtr fontName = desc->lookupFontName (pd->getFont ());
			if (fontName)
			{
				stringValue = fontName;
				return true;
			}
			return false;
		}
		else if (attributeName == "font-color")
		{
			if (!getRememberedAttributeValueString (view, "font-color", stringValue))
				colorToString (pd->getFontColor (), stringValue, desc);
			return true;
		}
		else if (attributeName == "back-color")
		{
			if (!getRememberedAttributeValueString (view, "back-color", stringValue))
				colorToString (pd->getBackColor (), stringValue, desc);
			return true;
		}
		else if (attributeName == "frame-color")
		{
			if (!getRememberedAttributeValueString (view, "frame-color", stringValue))
				colorToString (pd->getFrameColor (), stringValue, desc);
			return true;
		}
		else if (attributeName == "shadow-color")
		{
			if (!getRememberedAttributeValueString (view, "shadow-color", stringValue))
				colorToString (pd->getShadowColor (), stringValue, desc);
			return true;
		}
		else if (attributeName == "text-inset")
		{
			pointToString (pd->getTextInset (), stringValue);
			return true;
		}
		else if (attributeName == "font-antialias")
		{
			stringValue = pd->getAntialias () ? "true" : "false";
			return true;
		}
		else if (attributeName == "style-3D-in")
		{
			stringValue = pd->getStyle () & k3DIn ? "true" : "false";
			return true;
		}
		else if (attributeName == "style-3D-out")
		{
			stringValue = pd->getStyle () & k3DOut ? "true" : "false";
			return true;
		}
		else if (attributeName == "style-no-frame")
		{
			stringValue = pd->getStyle () & kNoFrame ? "true" : "false";
			return true;
		}
		else if (attributeName == "style-no-text")
		{
			stringValue = pd->getStyle () & kNoTextStyle ? "true" : "false";
			return true;
		}
		else if (attributeName == "style-no-draw")
		{
			stringValue = pd->getStyle () & kNoDrawStyle ? "true" : "false";
			return true;
		}
		else if (attributeName == "style-shadow-text")
		{
			stringValue = pd->getStyle () & kShadowText ? "true" : "false";
			return true;
		}
		else if (attributeName == "style-round-rect")
		{
			stringValue = pd->getStyle () & kRoundRectStyle ? "true" : "false";
			return true;
		}
		else if (attributeName == "round-rect-radius")
		{
			std::stringstream str;
			str << pd->getRoundRectRadius ();
			stringValue = str.str ();
			return true;
		}
		else if (attributeName == "frame-width")
		{
			std::stringstream str;
			str << pd->getFrameWidth ();
			stringValue = str.str ();
			return true;
		}
		else if (attributeName == "text-alignment")
		{
			CHoriTxtAlign align = pd->getHoriAlign ();
			switch (align)
			{
				case kLeftText: stringValue = "left"; break;
				case kRightText: stringValue = "right"; break;
				case kCenterText: stringValue = "center"; break;
			}
			return true;
		}
		else if (attributeName == "value-precision")
		{
			std::stringstream str;
			str << (uint32_t)pd->getPrecision ();
			stringValue = str.str ();
			return true;
		}
		return false;
	}

};
CParamDisplayCreator __gCParamDisplayCreator;

//-----------------------------------------------------------------------------
class COptionMenuCreator : public IViewCreator
{
public:
	COptionMenuCreator () { UIViewFactory::registerViewCreator (*this); }
	IdStringPtr getViewName () const { return "COptionMenu"; }
	IdStringPtr getBaseViewName () const { return "CParamDisplay"; }
	CView* create (const UIAttributes& attributes, IUIDescription* description) const { return new COptionMenu (); }
	bool apply (CView* view, const UIAttributes& attributes, IUIDescription* description) const
	{
		COptionMenu* menu = dynamic_cast<COptionMenu*> (view);
		if (!menu)
			return false;

		const std::string* attr = attributes.getAttributeValue ("menu-popup-style");
		if (attr)
		{
			if (*attr == "true")
				menu->setStyle (menu->getStyle () | kPopupStyle);
			else
				menu->setStyle (menu->getStyle () & ~kPopupStyle);
		}
		attr = attributes.getAttributeValue ("menu-check-style");
		if (attr)
		{
			if (*attr == "true")
				menu->setStyle (menu->getStyle () | kCheckStyle);
			else
				menu->setStyle (menu->getStyle () & ~kCheckStyle);
		}

		return true;
	}
	bool getAttributeNames (std::list<std::string>& attributeNames) const
	{
		attributeNames.push_back ("menu-popup-style");
		attributeNames.push_back ("menu-check-style");
		return true;
	}
	AttrType getAttributeType (const std::string& attributeName) const
	{
		if (attributeName == "menu-popup-style") return kBooleanType;
		if (attributeName == "menu-check-style") return kBooleanType;
		return kUnknownType;
	}
	bool getAttributeValue (CView* view, const std::string& attributeName, std::string& stringValue, IUIDescription* desc) const
	{
		COptionMenu* menu = dynamic_cast<COptionMenu*> (view);
		if (!menu)
			return false;
		if (attributeName == "menu-popup-style")
		{
			stringValue = (menu->getStyle () & kPopupStyle) ? "true" : "false";
			return true;
		}
		if (attributeName == "menu-check-style")
		{
			stringValue = (menu->getStyle () & kCheckStyle) ? "true" : "false";
			return true;
		}
		return false;
	}

};
COptionMenuCreator __gCOptionMenuCreator;

//-----------------------------------------------------------------------------
class CTextLabelCreator : public IViewCreator
{
public:
	CTextLabelCreator () { UIViewFactory::registerViewCreator (*this); }
	IdStringPtr getViewName () const { return "CTextLabel"; }
	IdStringPtr getBaseViewName () const { return "CParamDisplay"; }
	CView* create (const UIAttributes& attributes, IUIDescription* description) const { return new CTextLabel (CRect (0, 0, 100, 20)); }
	bool apply (CView* view, const UIAttributes& attributes, IUIDescription* description) const
	{
		CTextLabel* label = dynamic_cast<CTextLabel*> (view);
		if (!label)
			return false;

		const std::string* attr = attributes.getAttributeValue ("title");
		if (attr)
			label->setText (attr->c_str ());
		attr = attributes.getAttributeValue ("truncate-mode");
		if (attr)
		{
			if (*attr == "head")
				label->setTextTruncateMode (CTextLabel::kTruncateHead);
			else if (*attr == "tail")
				label->setTextTruncateMode (CTextLabel::kTruncateTail);
			else
				label->setTextTruncateMode (CTextLabel::kTruncateNone);
		}

		return true;
	}
	bool getAttributeNames (std::list<std::string>& attributeNames) const
	{
		attributeNames.push_back ("title");
		attributeNames.push_back ("truncate-mode");
		return true;
	}
	AttrType getAttributeType (const std::string& attributeName) const
	{
		if (attributeName == "title") return kStringType;
		if (attributeName == "truncate-mode") return kListType;
		return kUnknownType;
	}
	bool getAttributeValue (CView* view, const std::string& attributeName, std::string& stringValue, IUIDescription* desc) const
	{
		CTextLabel* label = dynamic_cast<CTextLabel*> (view);
		if (!label)
			return false;
		if (attributeName == "title")
		{
			UTF8StringPtr title = label->getText ();
			stringValue = title ? title : "";
			return true;
		}
		else if (attributeName == "truncate-mode")
		{
			switch (label->getTextTruncateMode ())
			{
				case CTextLabel::kTruncateHead: stringValue = "head"; break;
				case CTextLabel::kTruncateTail: stringValue = "tail"; break;
				case CTextLabel::kTruncateNone: stringValue = ""; break;
			}
			return true;
		}
		return false;
	}
	bool getPossibleListValues (const std::string& attributeName, std::list<const std::string*>& values) const
	{
		if (attributeName == "truncate-mode")
		{
			static std::string kNone = "none";
			static std::string kHead = "head";
			static std::string kTail = "tail";
	
			values.push_back (&kNone);
			values.push_back (&kHead);
			values.push_back (&kTail);
			return true;
		}
		return false;
	}

};
CTextLabelCreator __gCTextLabelCreator;

//-----------------------------------------------------------------------------
class CTextEditCreator : public IViewCreator
{
public:
	CTextEditCreator () { UIViewFactory::registerViewCreator (*this); }
	IdStringPtr getViewName () const { return "CTextEdit"; }
	IdStringPtr getBaseViewName () const { return "CTextLabel"; }
	CView* create (const UIAttributes& attributes, IUIDescription* description) const { return new CTextEdit (CRect (0, 0, 100, 20), 0, -1); }
	bool apply (CView* view, const UIAttributes& attributes, IUIDescription* description) const
	{
		CTextEdit* label = dynamic_cast<CTextEdit*> (view);
		if (!label)
			return false;

		const std::string* attr = attributes.getAttributeValue ("immediate-text-change");
		if (attr)
			label->setImmediateTextChange (*attr == "true" ? true : false);

		return true;
	}
	bool getAttributeNames (std::list<std::string>& attributeNames) const
	{
		attributeNames.push_back ("immediate-text-change");
		return true;
	}
	AttrType getAttributeType (const std::string& attributeName) const
	{
		if (attributeName == "immediate-text-change") return kBooleanType;
		return kUnknownType;
	}
	bool getAttributeValue (CView* view, const std::string& attributeName, std::string& stringValue, IUIDescription* desc) const
	{
		CTextEdit* label = dynamic_cast<CTextEdit*> (view);
		if (!label)
			return false;
		if (attributeName == "immediate-text-change")
		{
			stringValue = label->getImmediateTextChange () ? "true" : "false";
			return true;
		}
		
		return false;
	}

};
CTextEditCreator __gCTextEditCreator;

//-----------------------------------------------------------------------------
class CTextButtonCreator : public IViewCreator
{
public:
	CTextButtonCreator () { UIViewFactory::registerViewCreator (*this); }
	IdStringPtr getViewName () const { return "CTextButton"; }
	IdStringPtr getBaseViewName () const { return "CControl"; }
	CView* create (const UIAttributes& attributes, IUIDescription* description) const { return new CTextButton (CRect (0, 0, 100, 20), 0, -1, ""); }
	bool apply (CView* view, const UIAttributes& attributes, IUIDescription* description) const
	{
		CTextButton* button = dynamic_cast<CTextButton*> (view);
		if (!button)
			return false;

		const std::string* attr = attributes.getAttributeValue ("title");
		if (attr)
			button->setTitle (attr->c_str ());

		attr = attributes.getAttributeValue ("font");
		if (attr)
		{
			CFontRef font = description->getFont (attr->c_str ());
			if (font)
			{
				rememberAttributeValueString (view, "font", *attr);
				button->setFont (font);
			}
		}

		CColor color;
		attr = attributes.getAttributeValue ("text-color");
		if (attr)
		{
			if (description->getColor (attr->c_str (), color))
			{
				rememberAttributeValueString (view, "text-color", *attr);
				button->setTextColor (color);
			}
		}
		attr = attributes.getAttributeValue ("text-color-highlighted");
		if (attr)
		{
			if (description->getColor (attr->c_str (), color))
			{
				rememberAttributeValueString (view, "text-color-highlighted", *attr);
				button->setTextColorHighlighted (color);
			}
		}
		attr = attributes.getAttributeValue ("gradient-start-color");
		if (attr)
		{
			if (description->getColor (attr->c_str (), color))
			{
				rememberAttributeValueString (view, "gradient-start-color", *attr);
				button->setGradientStartColor (color);
			}
		}
		attr = attributes.getAttributeValue ("gradient-start-color-highlighted");
		if (attr)
		{
			if (description->getColor (attr->c_str (), color))
			{
				rememberAttributeValueString (view, "gradient-start-color-highlighted", *attr);
				button->setGradientStartColorHighlighted (color);
			}
		}
		attr = attributes.getAttributeValue ("gradient-end-color");
		if (attr)
		{
			if (description->getColor (attr->c_str (), color))
			{
				rememberAttributeValueString (view, "gradient-end-color", *attr);
				button->setGradientEndColor (color);
			}
		}
		attr = attributes.getAttributeValue ("gradient-end-color-highlighted");
		if (attr)
		{
			if (description->getColor (attr->c_str (), color))
			{
				rememberAttributeValueString (view, "gradient-end-color-highlighted", *attr);
				button->setGradientEndColorHighlighted (color);
			}
		}
		attr = attributes.getAttributeValue ("frame-color");
		if (attr)
		{
			if (description->getColor (attr->c_str (), color))
			{
				rememberAttributeValueString (view, "frame-color", *attr);
				button->setFrameColor (color);
			}
		}
		attr = attributes.getAttributeValue ("frame-color-highlighted");
		if (attr)
		{
			if (description->getColor (attr->c_str (), color))
			{
				rememberAttributeValueString (view, "frame-color-highlighted", *attr);
				button->setFrameColorHighlighted (color);
			}
		}
		attr = attributes.getAttributeValue ("frame-width");
		if (attr)
		{
			CCoord width = strtof (attr->c_str (), 0);
			button->setFrameWidth (width);
		}
		attr = attributes.getAttributeValue ("round-radius");
		if (attr)
		{
			CCoord width = strtof (attr->c_str (), 0);
			button->setRoundRadius (width);
		}
		attr = attributes.getAttributeValue ("kick-style");
		if (attr)
		{
			button->setStyle (*attr == "true" ? CTextButton::kKickStyle : CTextButton::kOnOffStyle);
		}
		attr = attributes.getAttributeValue ("icon");
		if (attr)
		{
			CBitmap* bitmap = description->getBitmap (attr->c_str ());
			button->setIcon (bitmap);
		}
		attr = attributes.getAttributeValue ("icon-highlighted");
		if (attr)
		{
			CBitmap* bitmap = description->getBitmap (attr->c_str ());
			button->setIconHighlighted (bitmap);
		}
		attr = attributes.getAttributeValue ("icon-position");
		if (attr)
		{
			CTextButton::IconPosition pos = CTextButton::kLeft;
			if (*attr == "left")
			{
				pos = CTextButton::kLeft;
			}
			else if (*attr == "right")
			{
				pos = CTextButton::kRight;
			}
			else if (*attr == "center above text")
			{
				pos = CTextButton::kCenterAbove;
			}
			else if (*attr == "center below text")
			{
				pos = CTextButton::kCenterBelow;
			}
			button->setIconPosition (pos);
		}
		double margin;
		if (attributes.getDoubleAttribute ("icon-text-margin", margin))
		{
			button->setTextMargin (margin);
		}
		attr = attributes.getAttributeValue ("text-alignment");
		if (attr)
		{
			CHoriTxtAlign align = kCenterText;
			if (*attr == "left")
				align = kLeftText;
			else if (*attr == "right")
				align = kRightText;
			button->setTextAlignment (align);
		}
		return true;
	}
	bool getAttributeNames (std::list<std::string>& attributeNames) const
	{
		attributeNames.push_back ("kick-style");
		attributeNames.push_back ("title");
		attributeNames.push_back ("font");
		attributeNames.push_back ("text-color");
		attributeNames.push_back ("text-color-highlighted");
		attributeNames.push_back ("gradient-start-color");
		attributeNames.push_back ("gradient-start-color-highlighted");
		attributeNames.push_back ("gradient-end-color");
		attributeNames.push_back ("gradient-end-color-highlighted");
		attributeNames.push_back ("frame-color");
		attributeNames.push_back ("frame-color-highlighted");
		attributeNames.push_back ("round-radius");
		attributeNames.push_back ("frame-width");
		attributeNames.push_back ("icon-text-margin");
		attributeNames.push_back ("text-alignment");
		attributeNames.push_back ("icon");
		attributeNames.push_back ("icon-highlighted");
		attributeNames.push_back ("icon-position");
		return true;
	}
	AttrType getAttributeType (const std::string& attributeName) const
	{
		if (attributeName == "title") return kStringType;
		if (attributeName == "font") return kFontType;
		if (attributeName == "text-color") return kColorType;
		if (attributeName == "text-color-highlighted") return kColorType;
		if (attributeName == "gradient-start-color") return kColorType;
		if (attributeName == "gradient-start-color-highlighted") return kColorType;
		if (attributeName == "gradient-end-color") return kColorType;
		if (attributeName == "gradient-end-color-highlighted") return kColorType;
		if (attributeName == "frame-color") return kColorType;
		if (attributeName == "frame-color-highlighted") return kColorType;
		if (attributeName == "frame-width") return kFloatType;
		if (attributeName == "round-radius") return kFloatType;
		if (attributeName == "kick-style") return kBooleanType;
		if (attributeName == "icon") return kBitmapType;
		if (attributeName == "icon-highlighted") return kBitmapType;
		if (attributeName == "icon-position") return kListType;
		if (attributeName == "icon-text-margin") return kFloatType;
		if (attributeName == "text-alignment") return kStringType;
		return kUnknownType;
	}
	bool getPossibleListValues (const std::string& attributeName, std::list<const std::string*>& values) const
	{
		if (attributeName == "icon-position")
		{
			static std::string positions[] = {
				"left",
				"center above text",
				"center below text",
				"right",
				""
			};
			int32_t index = 0;
			while (positions[index].size () > 0)
			{
				values.push_back(&positions[index]);
				index++;
			}
			return true;
		}
		return false;
	}
	bool getAttributeValue (CView* view, const std::string& attributeName, std::string& stringValue, IUIDescription* desc) const
	{
		CTextButton* button = dynamic_cast<CTextButton*> (view);
		if (!button)
			return false;
		if (attributeName == "title")
		{
			UTF8StringPtr title = button->getTitle ();
			stringValue = title ? title : "";
			return true;
		}
		else if (attributeName == "font")
		{
			if (getRememberedAttributeValueString (view, "font", stringValue))
				return true;
			UTF8StringPtr fontName = desc->lookupFontName (button->getFont ());
			if (fontName)
			{
				stringValue = fontName;
				return true;
			}
			return false;
		}
		else if (attributeName == "text-color")
		{
			if (!getRememberedAttributeValueString (view, "text-color", stringValue))
				colorToString (button->getTextColor (), stringValue, desc);
			return true;
		}
		else if (attributeName == "text-color-highlighted")
		{
			if (!getRememberedAttributeValueString (view, "text-color-highlighted", stringValue))
				colorToString (button->getTextColorHighlighted (), stringValue, desc);
			return true;
		}
		else if (attributeName == "gradient-start-color")
		{
			if (!getRememberedAttributeValueString (view, "gradient-start-color", stringValue))
				colorToString (button->getGradientStartColor (), stringValue, desc);
			return true;
		}
		else if (attributeName == "gradient-start-color-highlighted")
		{
			if (!getRememberedAttributeValueString (view, "gradient-start-color-highlighted", stringValue))
				colorToString (button->getGradientStartColorHighlighted (), stringValue, desc);
			return true;
		}
		else if (attributeName == "gradient-end-color")
		{
			if (!getRememberedAttributeValueString (view, "gradient-end-color", stringValue))
				colorToString (button->getGradientEndColor (), stringValue, desc);
			return true;
		}
		else if (attributeName == "gradient-end-color-highlighted")
		{
			if (!getRememberedAttributeValueString (view, "gradient-end-color-highlighted", stringValue))
				colorToString (button->getGradientEndColorHighlighted (), stringValue, desc);
			return true;
		}
		else if (attributeName == "frame-color")
		{
			if (!getRememberedAttributeValueString (view, "frame-color", stringValue))
				colorToString (button->getFrameColor (), stringValue, desc);
			return true;
		}
		else if (attributeName == "frame-color-highlighted")
		{
			if (!getRememberedAttributeValueString (view, "frame-color-highlighted", stringValue))
				colorToString (button->getFrameColorHighlighted (), stringValue, desc);
			return true;
		}
		else if (attributeName == "frame-width")
		{
			std::stringstream str;
			str << button->getFrameWidth ();
			stringValue = str.str ();
			return true;
		}
		else if (attributeName == "round-radius")
		{
			std::stringstream str;
			str << button->getRoundRadius ();
			stringValue = str.str ();
			return true;
		}
		else if (attributeName == "kick-style")
		{
			stringValue = button->getStyle() == CTextButton::kKickStyle ? "true" : "false";
			return true;
		}
		else if (attributeName == "icon")
		{
			CBitmap* bitmap = button->getIcon ();
			if (bitmap)
			{
				return bitmapToString (bitmap, stringValue, desc);
			}
		}
		else if (attributeName == "icon-highlighted")
		{
			CBitmap* bitmap = button->getIconHighlighted ();
			if (bitmap)
			{
				return bitmapToString (bitmap, stringValue, desc);
			}
		}
		else if (attributeName == "icon-position")
		{
			switch (button->getIconPosition ())
			{
				case CTextButton::kLeft:
				{
					stringValue = "left";
					break;
				}
				case CTextButton::kRight:
				{
					stringValue = "right";
					break;
				}
				case CTextButton::kCenterAbove:
				{
					stringValue = "center above text";
					break;
				}
				case CTextButton::kCenterBelow:
				{
					stringValue = "center below text";
					break;
				}
			}
			return true;
		}
		else if (attributeName == "icon-text-margin")
		{
			std::stringstream str;
			str << button->getTextMargin ();
			stringValue = str.str ();
			return true;
		}
		else if (attributeName == "text-alignment")
		{
			CHoriTxtAlign align = button->getTextAlignment ();
			switch (align)
			{
				case kLeftText: stringValue = "left"; break;
				case kRightText: stringValue = "right"; break;
				case kCenterText: stringValue = "center"; break;
			}
			return true;
		}
		return false;
	}

};
CTextButtonCreator __gCTextButtonCreator;

//-----------------------------------------------------------------------------
class CKnobCreator : public IViewCreator
{
public:
	CKnobCreator () { UIViewFactory::registerViewCreator (*this); }
	IdStringPtr getViewName () const { return "CKnob"; }
	IdStringPtr getBaseViewName () const { return "CControl"; }
	CView* create (const UIAttributes& attributes, IUIDescription* description) const { return new CKnob (CRect (0, 0, 0, 0), 0, -1, 0, 0); }
	bool apply (CView* view, const UIAttributes& attributes, IUIDescription* description) const
	{
		CKnob* knob = dynamic_cast<CKnob*> (view);
		if (!knob)
			return false;

		const std::string* angleStartAttr = attributes.getAttributeValue ("angle-start");
		const std::string* angleRangeAttr = attributes.getAttributeValue ("angle-range");
		const std::string* insetValueAttr = attributes.getAttributeValue ("value-inset");
		const std::string* coronaInsetAttr = attributes.getAttributeValue ("corona-inset");
		const std::string* zoomFactorAttr = attributes.getAttributeValue ("zoom-factor");
		const std::string* coronaColorAttr = attributes.getAttributeValue ("corona-color");
		const std::string* handleShadowColorAttr = attributes.getAttributeValue ("handle-shadow-color");
		const std::string* handleColorAttr = attributes.getAttributeValue ("handle-color");
		const std::string* handleBitmapAttr = attributes.getAttributeValue ("handle-bitmap");
		const std::string* handleLineWidthAttr = attributes.getAttributeValue ("handle-line-width");
		const std::string* circleDrawingAttr = attributes.getAttributeValue ("circle-drawing");
		const std::string* coronaDrawingAttr = attributes.getAttributeValue ("corona-drawing");
		const std::string* coronaFromCenterAttr = attributes.getAttributeValue ("corona-from-center");
		const std::string* coronaInvertedAttr = attributes.getAttributeValue ("corona-inverted");
		const std::string* coronaDashDotAttr = attributes.getAttributeValue ("corona-dash-dot");
		const std::string* coronaOutlineAttr = attributes.getAttributeValue ("corona-outline");

		float fvalue = 0.f;
		CColor color;
		if (angleStartAttr)
		{
			fvalue = strtof (angleStartAttr->c_str (), 0);
			// convert from degree
			fvalue = fvalue / 180.f * (float)kPI;
			knob->setStartAngle (fvalue);
		}
		if (angleRangeAttr)
		{
			fvalue = strtof (angleRangeAttr->c_str (), 0);
			// convert from degree
			fvalue = fvalue / 180.f * (float)kPI;
			knob->setRangeAngle (fvalue);
		}
		if (insetValueAttr)
		{
			fvalue = strtof (insetValueAttr->c_str (), 0);
			knob->setInsetValue (fvalue);
		}
		if (coronaInsetAttr)
		{
			fvalue = strtof (coronaInsetAttr->c_str (), 0);
			knob->setCoronaInset (fvalue);
		}
		if (zoomFactorAttr)
		{
			fvalue = strtof (zoomFactorAttr->c_str (), 0);
			knob->setZoomFactor (fvalue);
		}
		if (handleLineWidthAttr)
		{
			fvalue = strtof (handleLineWidthAttr->c_str (), 0);
			knob->setHandleLineWidth (fvalue);
		}
		if (coronaColorAttr)
		{
			if (description->getColor (coronaColorAttr->c_str (), color))
			{
				rememberAttributeValueString (view, "corona-color", *coronaColorAttr);
				knob->setCoronaColor (color);
			}
		}
		if (handleShadowColorAttr)
		{
			if (description->getColor (handleShadowColorAttr->c_str (), color))
			{
				rememberAttributeValueString (view, "handle-shadow-color", *handleShadowColorAttr);
				knob->setColorShadowHandle (color);
			}
		}
		if (handleColorAttr)
		{
			if (description->getColor (handleColorAttr->c_str (), color))
			{
				rememberAttributeValueString (view, "handle-color", *handleColorAttr);
				knob->setColorHandle (color);
			}
		}
		if (handleBitmapAttr && *handleBitmapAttr != "")
		{
			CBitmap* bitmap = description->getBitmap (handleBitmapAttr->c_str ());
			if (bitmap)
				knob->setHandleBitmap (bitmap);
		}
		int32_t drawStyle = knob->getDrawStyle ();
		if (circleDrawingAttr)
		{
			if (*circleDrawingAttr == "true")
				drawStyle |= CKnob::kHandleCircleDrawing;
			else
				drawStyle &= ~CKnob::kHandleCircleDrawing;
		}
		if (coronaDrawingAttr)
		{
			if (*coronaDrawingAttr == "true")
				drawStyle |= CKnob::kCoronaDrawing;
			else
				drawStyle &= ~CKnob::kCoronaDrawing;
		}
		if (coronaFromCenterAttr)
		{
			if (*coronaFromCenterAttr == "true")
				drawStyle |= CKnob::kCoronaFromCenter;
			else
				drawStyle &= ~CKnob::kCoronaFromCenter;
		}
		if (coronaInvertedAttr)
		{
			if (*coronaInvertedAttr == "true")
				drawStyle |= CKnob::kCoronaInverted;
			else
				drawStyle &= ~CKnob::kCoronaInverted;
		}
		if (coronaDashDotAttr)
		{
			if (*coronaDashDotAttr == "true")
				drawStyle |= CKnob::kCoronaLineDashDot;
			else
				drawStyle &= ~CKnob::kCoronaLineDashDot;
		}
		if (coronaOutlineAttr)
		{
			if (*coronaOutlineAttr == "true")
				drawStyle |= CKnob::kCoronaOutline;
			else
				drawStyle &= ~CKnob::kCoronaOutline;
		}
		knob->setDrawStyle (drawStyle);
		return true;
	}
	bool getAttributeNames (std::list<std::string>& attributeNames) const
	{
		attributeNames.push_back ("angle-start");
		attributeNames.push_back ("angle-range");
		attributeNames.push_back ("value-inset");
		attributeNames.push_back ("zoom-factor");
		attributeNames.push_back ("circle-drawing");
		attributeNames.push_back ("corona-drawing");
		attributeNames.push_back ("corona-outline");
		attributeNames.push_back ("corona-from-center");
		attributeNames.push_back ("corona-inverted");
		attributeNames.push_back ("corona-dash-dot");
		attributeNames.push_back ("corona-inset");
		attributeNames.push_back ("corona-color");
		attributeNames.push_back ("handle-shadow-color");
		attributeNames.push_back ("handle-color");
		attributeNames.push_back ("handle-line-width");
		attributeNames.push_back ("handle-bitmap");
		return true;
	}
	AttrType getAttributeType (const std::string& attributeName) const
	{
		if (attributeName == "angle-start") return kFloatType;
		else if (attributeName == "angle-range") return kFloatType;
		else if (attributeName == "value-inset") return kFloatType;
		else if (attributeName == "zoom-factor") return kFloatType;
		else if (attributeName == "circle-drawing") return kBooleanType;
		else if (attributeName == "corona-drawing") return kBooleanType;
		else if (attributeName == "corona-outline") return kBooleanType;
		else if (attributeName == "corona-from-center") return kBooleanType;
		else if (attributeName == "corona-inverted") return kBooleanType;
		else if (attributeName == "corona-dash-dot") return kBooleanType;
		else if (attributeName == "corona-inset") return kFloatType;
		else if (attributeName == "corona-color") return kColorType;
		else if (attributeName == "handle-shadow-color") return kColorType;
		else if (attributeName == "handle-color") return kColorType;
		else if (attributeName == "handle-line-width") return kFloatType;
		else if (attributeName == "handle-bitmap") return kBitmapType;
		return kUnknownType;
	}
	bool getAttributeValue (CView* view, const std::string& attributeName, std::string& stringValue, IUIDescription* desc) const
	{
		CKnob* knob = dynamic_cast<CKnob*> (view);
		if (!knob)
			return false;

		if (attributeName == "angle-start")
		{
			std::stringstream stream;
			stream << (knob->getStartAngle () / kPI * 180.);
			stringValue = stream.str ();
			return true;
		}
		else if (attributeName == "angle-range")
		{
			std::stringstream stream;
			stream << (knob->getRangeAngle () / kPI * 180.);
			stringValue = stream.str ();
			return true;
		}
		else if (attributeName == "value-inset")
		{
			std::stringstream stream;
			stream << knob->getInsetValue ();
			stringValue = stream.str ();
			return true;
		}
		else if (attributeName == "corona-inset")
		{
			std::stringstream stream;
			stream << knob->getCoronaInset ();
			stringValue = stream.str ();
			return true;
		}
		else if (attributeName == "zoom-factor")
		{
			std::stringstream stream;
			stream << knob->getZoomFactor ();
			stringValue = stream.str ();
			return true;
		}
		else if (attributeName == "handle-line-width")
		{
			std::stringstream stream;
			stream << knob->getHandleLineWidth ();
			stringValue = stream.str ();
			return true;
		}
		else if (attributeName == "corona-color")
		{
			if (!getRememberedAttributeValueString (view, "corona-color", stringValue))
				colorToString (knob->getCoronaColor (), stringValue, desc);
			return true;
		}
		else if (attributeName == "handle-shadow-color")
		{
			if (!getRememberedAttributeValueString (view, "handle-shadow-color", stringValue))
				colorToString (knob->getColorShadowHandle (), stringValue, desc);
			return true;
		}
		else if (attributeName == "handle-color")
		{
			if (!getRememberedAttributeValueString (view, "handle-color", stringValue))
				colorToString (knob->getColorHandle (), stringValue, desc);
			return true;
		}
		else if (attributeName == "handle-bitmap")
		{
			CBitmap* bitmap = knob->getHandleBitmap ();
			if (bitmap)
			{
				return bitmapToString (bitmap, stringValue, desc);
			}
		}
		else if (attributeName == "circle-drawing")
		{
			if (knob->getDrawStyle () & CKnob::kHandleCircleDrawing)
				stringValue = "true";
			else
				stringValue = "false";
			return true;
		}
		else if (attributeName == "corona-drawing")
		{
			if (knob->getDrawStyle () & CKnob::kCoronaDrawing)
				stringValue = "true";
			else
				stringValue = "false";
			return true;
		}
		else if (attributeName == "corona-from-center")
		{
			if (knob->getDrawStyle () & CKnob::kCoronaFromCenter)
				stringValue = "true";
			else
				stringValue = "false";
			return true;
		}
		else if (attributeName == "corona-inverted")
		{
			if (knob->getDrawStyle () & CKnob::kCoronaInverted)
				stringValue = "true";
			else
				stringValue = "false";
			return true;
		}
		else if (attributeName == "corona-dash-dot")
		{
			if (knob->getDrawStyle () & CKnob::kCoronaLineDashDot)
				stringValue = "true";
			else
				stringValue = "false";
			return true;
		}
		else if (attributeName == "corona-outline")
		{
			if (knob->getDrawStyle () & CKnob::kCoronaOutline)
				stringValue = "true";
			else
				stringValue = "false";
			return true;
		}
		return false;
	}

};
CKnobCreator __CKnobCreator;

//-----------------------------------------------------------------------------
class IMultiBitmapControlCreator : public IViewCreator
{
public:
	bool apply (CView* view, const UIAttributes& attributes, IUIDescription* description) const
	{
		IMultiBitmapControl* multiBitmapControl = dynamic_cast<IMultiBitmapControl*> (view);
		if (!multiBitmapControl)
			return false;

		const std::string* hightOfOneImageAttr = attributes.getAttributeValue ("height-of-one-image");
		if (hightOfOneImageAttr)
		{
			CCoord height = (CCoord)strtol (hightOfOneImageAttr->c_str (), 0, 10);
			multiBitmapControl->setHeightOfOneImage (height);
		}
		else
		{
			multiBitmapControl->autoComputeHeightOfOneImage ();
		}
		const std::string* attr = attributes.getAttributeValue ("sub-pixmaps");
		if (attr)
		{
			int32_t value = (int32_t)strtol (attr->c_str (), 0, 10);
			multiBitmapControl->setNumSubPixmaps (value);
		}
		return true;
	}
	bool getAttributeNames (std::list<std::string>& attributeNames) const
	{
		attributeNames.push_back ("height-of-one-image");
		attributeNames.push_back ("sub-pixmaps");
		return true;
	}
	AttrType getAttributeType (const std::string& attributeName) const
	{
		if (attributeName == "height-of-one-image") return kIntegerType;
		if (attributeName == "sub-pixmaps") return kIntegerType;
		return kUnknownType;
	}
	bool getAttributeValue (CView* view, const std::string& attributeName, std::string& stringValue, IUIDescription* desc) const
	{
		IMultiBitmapControl* multiBitmapControl = dynamic_cast<IMultiBitmapControl*> (view);
		if (!multiBitmapControl)
			return false;

		if (attributeName == "height-of-one-image")
		{
			std::stringstream stream;
			stream << multiBitmapControl->getHeightOfOneImage ();
			stringValue = stream.str ();
			return true;
		}
		if (attributeName == "sub-pixmaps")
		{
			std::stringstream stream;
			stream << multiBitmapControl->getNumSubPixmaps ();
			stringValue = stream.str ();
			return true;
		}
		return false;
	}

};

//-----------------------------------------------------------------------------
class CAnimKnobCreator : public IMultiBitmapControlCreator
{
public:
	CAnimKnobCreator () { UIViewFactory::registerViewCreator (*this); }
	IdStringPtr getViewName () const { return "CAnimKnob"; }
	IdStringPtr getBaseViewName () const { return "CKnob"; }
	CView* create (const UIAttributes& attributes, IUIDescription* description) const { return new CAnimKnob (CRect (0, 0, 0, 0), 0, -1, 0); }
};
CAnimKnobCreator __gCAnimKnobCreator;

//-----------------------------------------------------------------------------
class CVerticalSwitchCreator : public IMultiBitmapControlCreator
{
public:
	CVerticalSwitchCreator () { UIViewFactory::registerViewCreator (*this); }
	IdStringPtr getViewName () const { return "CVerticalSwitch"; }
	IdStringPtr getBaseViewName () const { return "CControl"; }
	CView* create (const UIAttributes& attributes, IUIDescription* description) const { return new CVerticalSwitch (CRect (0, 0, 0, 0), 0, -1, 0); }
};
CVerticalSwitchCreator __gCVerticalSwitchCreator;

//-----------------------------------------------------------------------------
class CHorizontalSwitchCreator : public IMultiBitmapControlCreator
{
public:
	CHorizontalSwitchCreator () { UIViewFactory::registerViewCreator (*this); }
	IdStringPtr getViewName () const { return "CHorizontalSwitch"; }
	IdStringPtr getBaseViewName () const { return "CControl"; }
	CView* create (const UIAttributes& attributes, IUIDescription* description) const { return new CHorizontalSwitch (CRect (0, 0, 0, 0), 0, -1, 0); }
};
CHorizontalSwitchCreator __gCHorizontalSwitchCreator;

//-----------------------------------------------------------------------------
class CRockerSwitchCreator : public IMultiBitmapControlCreator
{
public:
	CRockerSwitchCreator () { UIViewFactory::registerViewCreator (*this); }
	IdStringPtr getViewName () const { return "CRockerSwitch"; }
	IdStringPtr getBaseViewName () const { return "CControl"; }
	CView* create (const UIAttributes& attributes, IUIDescription* description) const { return new CRockerSwitch (CRect (0, 0, 0, 0), 0, -1, 0); }
};
CRockerSwitchCreator __gCRockerSwitchCreator;

//-----------------------------------------------------------------------------
class CMovieBitmapCreator : public IMultiBitmapControlCreator
{
public:
	CMovieBitmapCreator () { UIViewFactory::registerViewCreator (*this); }
	IdStringPtr getViewName () const { return "CMovieBitmap"; }
	IdStringPtr getBaseViewName () const { return "CControl"; }
	CView* create (const UIAttributes& attributes, IUIDescription* description) const { return new CMovieBitmap (CRect (0, 0, 0, 0), 0, -1, 0); }
};
CMovieBitmapCreator __gCMovieBitmapCreator;

//-----------------------------------------------------------------------------
class CMovieButtonCreator : public IMultiBitmapControlCreator
{
public:
	CMovieButtonCreator () { UIViewFactory::registerViewCreator (*this); }
	IdStringPtr getViewName () const { return "CMovieButton"; }
	IdStringPtr getBaseViewName () const { return "CControl"; }
	CView* create (const UIAttributes& attributes, IUIDescription* description) const { return new CMovieButton (CRect (0, 0, 0, 0), 0, -1, 0); }
};
CMovieButtonCreator __gCMovieButtonCreator;

//-----------------------------------------------------------------------------
class CKickButtonCreator : public IMultiBitmapControlCreator
{
public:
	CKickButtonCreator () { UIViewFactory::registerViewCreator (*this); }
	IdStringPtr getViewName () const { return "CKickButton"; }
	IdStringPtr getBaseViewName () const { return "CControl"; }
	CView* create (const UIAttributes& attributes, IUIDescription* description) const { return new CKickButton (CRect (0, 0, 0, 0), 0, -1, 0); }
};
CKickButtonCreator __gCKickButtonCreator;

//-----------------------------------------------------------------------------
class CSliderCreator : public IViewCreator
{
public:
	CSliderCreator () { UIViewFactory::registerViewCreator (*this); }
	IdStringPtr getViewName () const { return "CSlider"; }
	IdStringPtr getBaseViewName () const { return "CControl"; }
	CView* create (const UIAttributes& attributes, IUIDescription* description) const { return new CSlider (CRect (0, 0, 0, 0), 0, -1, 0, 0, 0, 0); }
	bool apply (CView* view, const UIAttributes& attributes, IUIDescription* description) const
	{
		CSlider* slider = dynamic_cast<CSlider*> (view);
		if (!slider)
			return false;

		const std::string* transparentHandleAttr = attributes.getAttributeValue ("transparent-handle");
		const std::string* modeAttr = attributes.getAttributeValue ("mode");
		const std::string* handleBitmapAttr = attributes.getAttributeValue ("handle-bitmap");
		const std::string* handleOffsetAttr = attributes.getAttributeValue ("handle-offset");
		const std::string* bitmapOffsetAttr = attributes.getAttributeValue ("bitmap-offset");
		const std::string* zoomFactorAttr = attributes.getAttributeValue ("zoom-factor");
		const std::string* orientationAttr = attributes.getAttributeValue ("orientation");
		const std::string* reverseOrientationAttr = attributes.getAttributeValue ("reverse-orientation");
		const std::string* drawFrameAttr = attributes.getAttributeValue ("draw-frame");
		const std::string* drawBackAttr = attributes.getAttributeValue ("draw-back");
		const std::string* drawValueAttr = attributes.getAttributeValue ("draw-value");
		const std::string* drawValueFromCenterAttr = attributes.getAttributeValue ("draw-value-from-center");
		const std::string* drawValueInvertedAttr = attributes.getAttributeValue ("draw-value-inverted");
		const std::string* drawFrameColorAttr = attributes.getAttributeValue ("draw-frame-color");
		const std::string* drawBackColorAttr = attributes.getAttributeValue ("draw-back-color");
		const std::string* drawValueColorAttr = attributes.getAttributeValue ("draw-value-color");

		// support old attribute name and convert it
		const std::string* freeClickAttr = attributes.getAttributeValue ("free-click");
		if (freeClickAttr)
		{
			slider->setMode (*freeClickAttr == "true" ? CSlider::kFreeClickMode : CSlider::kTouchMode);
		}


		CPoint p;
		if (transparentHandleAttr)
			slider->setDrawTransparentHandle (*transparentHandleAttr == "true");
		if (modeAttr)
		{
			if (*modeAttr == "touch")
				slider->setMode (CSlider::kTouchMode);
			else if (*modeAttr == "relative touch")
				slider->setMode (CSlider::kRelativeTouchMode);
			else if (*modeAttr == "free click")
				slider->setMode (CSlider::kFreeClickMode);
		}
		if (handleBitmapAttr && *handleBitmapAttr != "")
		{
			CBitmap* bitmap = description->getBitmap (handleBitmapAttr->c_str ());
			if (bitmap)
				slider->setHandle (bitmap);
		}
		if (handleOffsetAttr)
		{
			if (parseSize (*handleOffsetAttr, p))
				slider->setOffsetHandle (p);
		}
		if (bitmapOffsetAttr)
		{
			if (parseSize (*bitmapOffsetAttr, p))
				slider->setOffset (p);
		}
		if (zoomFactorAttr)
		{
			float zoomFactor = strtof (zoomFactorAttr->c_str (), 0);
			slider->setZoomFactor (zoomFactor);
		}
		if (orientationAttr)
		{
			int32_t style = slider->getStyle ();
			if (*orientationAttr == "vertical")
			{
				style &= ~kHorizontal;
				style |= kVertical;
			}
			else
			{
				style &= ~kVertical;
				style |= kHorizontal;
			}
			slider->setStyle (style);
		}
		if (reverseOrientationAttr)
		{
			int32_t style = slider->getStyle ();
			if (*reverseOrientationAttr == "true")
			{
				if (style & kVertical)
				{
					style &= ~kBottom;
					style |= kTop;
				}
				else if (style & kHorizontal)
				{
					style &= ~kLeft;
					style |= kRight;
				}
			}
			else
			{
				if (style & kVertical)
				{
					style &= ~kTop;
					style |= kBottom;
				}
				else if (style & kHorizontal)
				{
					style &= ~kRight;
					style |= kLeft;
				}
			}
			slider->setStyle (style);
		}
		int32_t drawStyle = slider->getDrawStyle ();
		if (drawFrameAttr)
		{
			if (*drawFrameAttr == "true")
				drawStyle |= CSlider::kDrawFrame;
			else
				drawStyle &= ~CSlider::kDrawFrame;
		}
		if (drawBackAttr)
		{
			if (*drawBackAttr == "true")
				drawStyle |= CSlider::kDrawBack;
			else
				drawStyle &= ~CSlider::kDrawBack;
		}
		if (drawValueAttr)
		{
			if (*drawValueAttr == "true")
				drawStyle |= CSlider::kDrawValue;
			else
				drawStyle &= ~CSlider::kDrawValue;
		}
		if (drawValueFromCenterAttr)
		{
			if (*drawValueFromCenterAttr == "true")
				drawStyle |= CSlider::kDrawValueFromCenter;
			else
				drawStyle &= ~CSlider::kDrawValueFromCenter;
		}
		if (drawValueInvertedAttr)
		{
			if (*drawValueInvertedAttr == "true")
				drawStyle |= CSlider::kDrawInverted;
			else
				drawStyle &= ~CSlider::kDrawInverted;
		}
		slider->setDrawStyle (drawStyle);
		CColor color;
		if (drawFrameColorAttr)
		{
			if (description->getColor (drawFrameColorAttr->c_str (), color))
			{
				rememberAttributeValueString (view, "draw-frame-color", *drawFrameColorAttr);
				slider->setFrameColor (color);
			}
		}
		if (drawBackColorAttr)
		{
			if (description->getColor (drawBackColorAttr->c_str (), color))
			{
				rememberAttributeValueString (view, "draw-back-color", *drawBackColorAttr);
				slider->setBackColor (color);
			}
		}
		if (drawValueColorAttr)
		{
			if (description->getColor (drawValueColorAttr->c_str (), color))
			{
				rememberAttributeValueString (view, "draw-value-color", *drawValueColorAttr);
				slider->setValueColor (color);
			}
		}
		return true;
	}
	bool getAttributeNames (std::list<std::string>& attributeNames) const
	{
		attributeNames.push_back ("transparent-handle");
		attributeNames.push_back ("mode");
		attributeNames.push_back ("handle-bitmap");
		attributeNames.push_back ("handle-offset");
		attributeNames.push_back ("bitmap-offset");
		attributeNames.push_back ("zoom-factor");
		attributeNames.push_back ("orientation");
		attributeNames.push_back ("reverse-orientation");
		attributeNames.push_back ("draw-frame");
		attributeNames.push_back ("draw-back");
		attributeNames.push_back ("draw-value");
		attributeNames.push_back ("draw-value-from-center");
		attributeNames.push_back ("draw-value-inverted");
		attributeNames.push_back ("draw-frame-color");
		attributeNames.push_back ("draw-back-color");
		attributeNames.push_back ("draw-value-color");
		return true;
	}
	AttrType getAttributeType (const std::string& attributeName) const
	{
		if (attributeName == "transparent-handle") return kBooleanType;
		if (attributeName == "mode") return kListType;
		if (attributeName == "handle-bitmap") return kBitmapType;
		if (attributeName == "handle-offset") return kPointType;
		if (attributeName == "bitmap-offset") return kPointType;
		if (attributeName == "zoom-factor") return kFloatType;
		if (attributeName == "orientation") return kListType;
		if (attributeName == "reverse-orientation") return kBooleanType;
		if (attributeName == "draw-frame") return kBooleanType;
		if (attributeName == "draw-back") return kBooleanType;
		if (attributeName == "draw-value") return kBooleanType;
		if (attributeName == "draw-value-from-center") return kBooleanType;
		if (attributeName == "draw-value-inverted") return kBooleanType;
		if (attributeName == "draw-frame-color") return kColorType;
		if (attributeName == "draw-back-color") return kColorType;
		if (attributeName == "draw-value-color") return kColorType;
		return kUnknownType;
	}
	bool getAttributeValue (CView* view, const std::string& attributeName, std::string& stringValue, IUIDescription* desc) const
	{
		CSlider* slider = dynamic_cast<CSlider*> (view);
		if (!slider)
			return false;
		if (attributeName == "transparent-handle")
		{
			stringValue = slider->getDrawTransparentHandle () ? "true" : "false";
			return true;
		}
		else if (attributeName == "mode")
		{
			switch (slider->getMode ())
			{
				case CSlider::kTouchMode:
					stringValue = "touch"; break;
				case CSlider::kRelativeTouchMode:
					stringValue = "relative touch"; break;
				case CSlider::kFreeClickMode:
					stringValue = "free click"; break;
			}
			return true;
		}
		else if (attributeName == "handle-bitmap")
		{
			CBitmap* bitmap = slider->getHandle ();
			if (bitmap)
			{
				bitmapToString (bitmap, stringValue, desc);
			}
			return true;
		}
		else if (attributeName == "handle-offset")
		{
			pointToString (slider->getOffsetHandle (), stringValue);
			return true;
		}
		else if (attributeName == "bitmap-offset")
		{
			pointToString (slider->getBackOffset (), stringValue);
			return true;
		}
		else if (attributeName == "zoom-factor")
		{
			std::stringstream stream;
			stream << slider->getZoomFactor ();
			stringValue = stream.str ();
			return true;
		}
		else if (attributeName == "orientation")
		{
			if (slider->getStyle () & kVertical)
				stringValue = "vertical";
			else
				stringValue = "horizontal";
			return true;
		}
		else if (attributeName == "reverse-orientation")
		{
			int32_t style = slider->getStyle ();
			stringValue = "false";
			if (((style & kVertical) && (style & kTop)) || ((style & kHorizontal) && (style & kRight)))
				stringValue = "true";
			else
				stringValue = "false";
			return true;
		}
		else if (attributeName == "draw-frame")
		{
			if (slider->getDrawStyle () & CSlider::kDrawFrame)
				stringValue = "true";
			else
				stringValue = "false";
			return true;
		}
		else if (attributeName == "draw-back")
		{
			if (slider->getDrawStyle () & CSlider::kDrawBack)
				stringValue = "true";
			else
				stringValue = "false";
			return true;
		}
		else if (attributeName == "draw-value")
		{
			if (slider->getDrawStyle () & CSlider::kDrawValue)
				stringValue = "true";
			else
				stringValue = "false";
			return true;
		}
		else if (attributeName == "draw-value-from-center")
		{
			if (slider->getDrawStyle () & CSlider::kDrawValueFromCenter)
				stringValue = "true";
			else
				stringValue = "false";
			return true;
		}
		else if (attributeName == "draw-value-inverted")
		{
			if (slider->getDrawStyle () & CSlider::kDrawInverted)
				stringValue = "true";
			else
				stringValue = "false";
			return true;
		}
		else if (attributeName == "draw-frame-color")
		{
			if (!getRememberedAttributeValueString (view, "draw-frame-color", stringValue))
				colorToString (slider->getFrameColor (), stringValue, desc);
			return true;
		}
		else if (attributeName == "draw-back-color")
		{
			if (!getRememberedAttributeValueString (view, "draw-back-color", stringValue))
				colorToString (slider->getBackColor (), stringValue, desc);
			return true;
		}
		else if (attributeName == "draw-value-color")
		{
			if (!getRememberedAttributeValueString (view, "draw-value-color", stringValue))
				colorToString (slider->getValueColor (), stringValue, desc);
			return true;
		}

		return false;
	}
	bool getPossibleListValues (const std::string& attributeName, std::list<const std::string*>& values) const
	{
		if (attributeName == "orientation")
		{
			static std::string kHorizontal = "horizontal";
			static std::string kVertical = "vertical";
	
			values.push_back (&kHorizontal);
			values.push_back (&kVertical);
			return true;
		}
		if (attributeName == "mode")
		{
			static std::string kTouch = "touch";
			static std::string kRelativeTouch = "relative touch";
			static std::string kFreeClick = "free click";
			
			values.push_back (&kTouch);
			values.push_back (&kRelativeTouch);
			values.push_back (&kFreeClick);
			return true;
		}
		return false;
	}

};
CSliderCreator __gCSliderCreator;

//-----------------------------------------------------------------------------
class CVuMeterCreator : public IViewCreator
{
public:
	CVuMeterCreator () { UIViewFactory::registerViewCreator (*this); }
	IdStringPtr getViewName () const { return "CVuMeter"; }
	IdStringPtr getBaseViewName () const { return "CControl"; }
	CView* create (const UIAttributes& attributes, IUIDescription* description) const { return new CVuMeter (CRect (0, 0, 0, 0), 0, 0, 100); }
	bool apply (CView* view, const UIAttributes& attributes, IUIDescription* description) const
	{
		CVuMeter* vuMeter = dynamic_cast<CVuMeter*> (view);
		if (!vuMeter)
			return false;

		const std::string* attr = attributes.getAttributeValue ("off-bitmap");
		if (attr && *attr != "")
		{
			CBitmap* bitmap = description->getBitmap (attr->c_str ());
			vuMeter->setOffBitmap (bitmap);
		}
		attr = attributes.getAttributeValue ("orientation");
		if (attr)
		{
			vuMeter->setStyle (*attr == "vertical" ? kVertical : kHorizontal);
		}
		attr = attributes.getAttributeValue ("num-led");
		if (attr)
		{
			int32_t numLed = (int32_t)strtol (attr->c_str (), 0, 10);
			vuMeter->setNbLed (numLed);
		}
		attr = attributes.getAttributeValue ("decrease-step-value");
		if (attr)
		{
			float value = strtof (attr->c_str (), 0);
			vuMeter->setDecreaseStepValue (value);
		}
		return true;
	}
	bool getAttributeNames (std::list<std::string>& attributeNames) const
	{
		attributeNames.push_back ("off-bitmap");
		attributeNames.push_back ("num-led");
		attributeNames.push_back ("orientation");
		attributeNames.push_back ("decrease-step-value");
		return true;
	}
	AttrType getAttributeType (const std::string& attributeName) const
	{
		if (attributeName == "off-bitmap") return kBitmapType;
		if (attributeName == "num-led") return kIntegerType;
		if (attributeName == "orientation") return kListType;
		if (attributeName == "decrease-step-value") return kFloatType;
		return kUnknownType;
	}
	bool getAttributeValue (CView* view, const std::string& attributeName, std::string& stringValue, IUIDescription* desc) const
	{
		CVuMeter* vuMeter = dynamic_cast<CVuMeter*> (view);
		if (!vuMeter)
			return false;
		if (attributeName == "off-bitmap")
		{
			CBitmap* bitmap = vuMeter->getOffBitmap ();
			if (bitmap)
			{
				bitmapToString (bitmap, stringValue, desc);
			}
			return true;
		}
		else if (attributeName == "orientation")
		{
			if (vuMeter->getStyle () & kVertical)
				stringValue = "vertical";
			else
				stringValue = "horizontal";
			return true;
		}
		else if (attributeName == "num-led")
		{
			std::stringstream stream;
			stream << vuMeter->getNbLed ();
			stringValue = stream.str ();
			return true;
		}
		else if (attributeName == "decrease-step-value")
		{
			std::stringstream stream;
			stream << vuMeter->getDecreaseStepValue ();
			stringValue = stream.str ();
			return true;
		}
		return false;
	}
	bool getPossibleListValues (const std::string& attributeName, std::list<const std::string*>& values) const
	{
		if (attributeName == "orientation")
		{
			static std::string kHorizontal = "horizontal";
			static std::string kVertical = "vertical";
	
			values.push_back (&kHorizontal);
			values.push_back (&kVertical);
			return true;
		}
		return false;
	}

};
CVuMeterCreator __gCVuMeterCreator;

//-----------------------------------------------------------------------------
class CAnimationSplashScreenCreator : public IViewCreator
{
public:
	CAnimationSplashScreenCreator () { UIViewFactory::registerViewCreator (*this); }
	IdStringPtr getViewName () const { return "CAnimationSplashScreen"; }
	IdStringPtr getBaseViewName () const { return "CControl"; }
	CView* create (const UIAttributes& attributes, IUIDescription* description) const { return new CAnimationSplashScreen (CRect (0, 0, 0, 0), -1, 0, 0); }
	bool apply (CView* view, const UIAttributes& attributes, IUIDescription* description) const
	{
		CAnimationSplashScreen* splashScreen = dynamic_cast<CAnimationSplashScreen*> (view);
		if (!splashScreen)
			return false;

		const std::string* attr = attributes.getAttributeValue ("splash-bitmap");
		if (attr && *attr != "")
		{
			CBitmap* bitmap = description->getBitmap (attr->c_str ());
			splashScreen->setSplashBitmap (bitmap);
		}
		attr = attributes.getAttributeValue ("splash-origin");
		CPoint p;
		if (attr)
		{
			if (parseSize (*attr, p))
			{
				CRect size = splashScreen->getSplashRect ();
				size.originize ();
				size.offset (p.x, p.y);
				splashScreen->setSplashRect (size);
			}
		}
		attr = attributes.getAttributeValue ("splash-size");
		if (attr)
		{
			if (parseSize (*attr, p))
			{
				CRect size = splashScreen->getSplashRect ();
				size.setWidth (p.x);
				size.setHeight (p.y);
				splashScreen->setSplashRect (size);
			}
		}
		attr = attributes.getAttributeValue ("animation-index");
		if (attr)
		{
			int32_t index = (int32_t)strtol (attr->c_str (), 0, 10);
			splashScreen->setAnimationIndex (index);
		}
		attr = attributes.getAttributeValue ("animation-time");
		if (attr)
		{
			int32_t time = (int32_t)strtol (attr->c_str (), 0, 10);
			splashScreen->setAnimationTime (time);
		}

		return true;
	}
	bool getAttributeNames (std::list<std::string>& attributeNames) const
	{
		attributeNames.push_back ("splash-bitmap");
		attributeNames.push_back ("splash-origin");
		attributeNames.push_back ("splash-size");
		attributeNames.push_back ("animation-index");
		attributeNames.push_back ("animation-time");
		return true;
	}
	AttrType getAttributeType (const std::string& attributeName) const
	{
		if (attributeName == "splash-bitmap") return kBitmapType;
		if (attributeName == "splash-origin") return kRectType;
		if (attributeName == "splash-size") return kRectType;
		if (attributeName == "animation-index") return kIntegerType;
		if (attributeName == "animation-time") return kIntegerType;
		return kUnknownType;
	}
	bool getAttributeValue (CView* view, const std::string& attributeName, std::string& stringValue, IUIDescription* desc) const
	{
		CAnimationSplashScreen* splashScreen = dynamic_cast<CAnimationSplashScreen*> (view);
		if (!splashScreen)
			return false;

		if (attributeName == "splash-bitmap")
		{
			CBitmap* bitmap = splashScreen->getSplashBitmap ();
			if (bitmap)
				bitmapToString (bitmap, stringValue, desc);
			else
				stringValue = "";
			return true;
		}
		else if (attributeName == "splash-origin")
		{
			pointToString (splashScreen->getSplashRect ().getTopLeft (), stringValue);
			return true;
		}
		else if (attributeName == "splash-size")
		{
			pointToString (splashScreen->getSplashRect ().getSize (), stringValue);
			return true;
		}
		else if (attributeName == "animation-index")
		{
			std::stringstream stream;
			stream << splashScreen->getAnimationIndex ();
			stringValue = stream.str ();
			return true;
		}
		else if (attributeName == "animation-time")
		{
			std::stringstream stream;
			stream << splashScreen->getAnimationTime ();
			stringValue = stream.str ();
			return true;
		}
		return false;
	}

};
CAnimationSplashScreenCreator __gCAnimationSplashScreenCreator;

//-----------------------------------------------------------------------------
class UIViewSwitchContainerCreator : public IViewCreator
{
public:
	UIViewSwitchContainerCreator () { UIViewFactory::registerViewCreator (*this); }
	IdStringPtr getViewName () const { return "UIViewSwitchContainer"; }
	IdStringPtr getBaseViewName () const { return "CViewContainer"; }
	CView* create (const UIAttributes& attributes, IUIDescription* description) const 
	{
		UIViewSwitchContainer* vsc = new UIViewSwitchContainer (CRect (0, 0, 100, 100));
		new UIDescriptionViewSwitchController (vsc, dynamic_cast<UIDescription*> (description), description->getController ());
		return vsc;
	}

	bool apply (CView* view, const UIAttributes& attributes, IUIDescription* description) const
	{
		UIViewSwitchContainer* viewSwitch = dynamic_cast<UIViewSwitchContainer*> (view);
		if (!viewSwitch)
			return false;
		const std::string* attr = attributes.getAttributeValue ("template-names");
		if (attr)
		{
			UIDescriptionViewSwitchController* controller = dynamic_cast<UIDescriptionViewSwitchController*> (viewSwitch->getController ());
			if (controller)
			{
				controller->setTemplateNames (attr->c_str ());
			}
		}
		attr = attributes.getAttributeValue ("template-switch-control");
		if (attr)
		{
			rememberAttributeValueString (view, "template-switch-control", *attr);
			UIDescriptionViewSwitchController* controller = dynamic_cast<UIDescriptionViewSwitchController*> (viewSwitch->getController ());
			if (controller)
			{
				int32_t tag = description->getTagForName (attr->c_str ());
				controller->setSwitchControlTag (tag);
			}
		}
		int32_t animationTime;
		if (attributes.getIntegerAttribute ("animation-time", animationTime))
		{
			viewSwitch->setAnimationTime (animationTime);
		}
		return true;
	}
	bool getAttributeNames (std::list<std::string>& attributeNames) const
	{
		attributeNames.push_back ("template-names");
		attributeNames.push_back ("template-switch-control");
		attributeNames.push_back ("animation-time");
		return true;
	}
	AttrType getAttributeType (const std::string& attributeName) const
	{
		if (attributeName == "template-names") return kStringType;
		if (attributeName == "template-switch-control") return kTagType;
		if (attributeName == "animation-time") return kIntegerType;
		return kUnknownType;
	}
	bool getAttributeValue (CView* view, const std::string& attributeName, std::string& stringValue, IUIDescription* desc) const
	{
		UIViewSwitchContainer* viewSwitch = dynamic_cast<UIViewSwitchContainer*> (view);
		if (!viewSwitch)
			return false;
		if (attributeName == "template-names")
		{
			UIDescriptionViewSwitchController* controller = dynamic_cast<UIDescriptionViewSwitchController*> (viewSwitch->getController ());
			if (controller)
			{
				controller->getTemplateNames (stringValue);
				return true;
			}
		}
		else if (attributeName == "template-switch-control")
		{
			UIDescriptionViewSwitchController* controller = dynamic_cast<UIDescriptionViewSwitchController*> (viewSwitch->getController ());
			if (controller)
			{
				if (getRememberedAttributeValueString (view, "template-switch-control", stringValue))
					return true;
				UTF8StringPtr controlTag = desc->lookupControlTagName (controller->getSwitchControlTag ());
				if (controlTag)
				{
					stringValue = controlTag;
					return true;
				}
				return true;
			}
		}
		else if (attributeName == "animation-time")
		{
			std::stringstream stream;
			stream << (int32_t)viewSwitch->getAnimationTime ();
			stringValue = stream.str ();
			return true;
		}
		return false;
	}
};
UIViewSwitchContainerCreator __gUIViewSwitchContainerCreator;

//-----------------------------------------------------------------------------
class CSplitViewCreator : public IViewCreator
{
public:
	CSplitViewCreator () { UIViewFactory::registerViewCreator (*this); }
	IdStringPtr getViewName () const { return "CSplitView"; }
	IdStringPtr getBaseViewName () const { return "CViewContainer"; }
	CView* create (const UIAttributes& attributes, IUIDescription* description) const { return new CSplitView (CRect (0, 0, 100, 100)); }

	bool apply (CView* view, const UIAttributes& attributes, IUIDescription* description) const
	{
		CSplitView* splitView = dynamic_cast<CSplitView*> (view);
		if (!splitView)
			return false;

		const std::string* attr = attributes.getAttributeValue ("separator-width");
		if (attr)
		{
			int32_t width = (int32_t)strtol (attr->c_str (), 0, 10);
			splitView->setSeparatorWidth (width);
		}
		attr = attributes.getAttributeValue ("orientation");
		if (attr)
		{
			if (*attr == "horizontal")
			{
				splitView->setStyle (CSplitView::kHorizontal);
			}
			else
			{
				splitView->setStyle (CSplitView::kVertical);
			}
		}
		attr = attributes.getAttributeValue ("resize-method");
		if (attr)
		{
			if (*attr == "first")
			{
				splitView->setResizeMethod (CSplitView::kResizeFirstView);
			}
			if (*attr == "second")
			{
				splitView->setResizeMethod (CSplitView::kResizeSecondView);
			}
			else if (*attr == "last")
			{
				splitView->setResizeMethod (CSplitView::kResizeLastView);
			}
			else if (*attr == "all")
			{
				splitView->setResizeMethod (CSplitView::kResizeAllViews);
			}
		}

		return true;
	}
	bool getAttributeNames (std::list<std::string>& attributeNames) const
	{
		attributeNames.push_back ("orientation");
		attributeNames.push_back ("resize-method");
		attributeNames.push_back ("separator-width");
		return true;
	}
	AttrType getAttributeType (const std::string& attributeName) const
	{
		if (attributeName == "orientation") return kListType;
		if (attributeName == "resize-method") return kListType;
		if (attributeName == "separator-width") return kIntegerType;
		return kUnknownType;
	}
	bool getAttributeValue (CView* view, const std::string& attributeName, std::string& stringValue, IUIDescription* desc) const
	{
		CSplitView* splitView = dynamic_cast<CSplitView*> (view);
		if (!splitView)
			return false;
		if (attributeName == "separator-width")
		{
//			std::basic_ostream<char>
			std::stringstream stream;
			stream << (int32_t)splitView->getSeparatorWidth ();
			stringValue = stream.str ();
			return true;
		}
		if (attributeName == "orientation")
		{
			stringValue = splitView->getStyle () == CSplitView::kHorizontal ? "horizontal" : "vertical";
			return true;
		}
		if (attributeName == "resize-method")
		{
			switch (splitView->getResizeMethod ())
			{
				case CSplitView::kResizeFirstView:
				{
					stringValue = "first";
					return true;
				}
				case CSplitView::kResizeSecondView:
				{
					stringValue = "second";
					return true;
				}
				case CSplitView::kResizeLastView:
				{
					stringValue = "last";
					return true;
				}
				case CSplitView::kResizeAllViews:
				{
					stringValue = "all";
					return true;
				}
			}
		}
		return false;
	}
	bool getPossibleListValues (const std::string& attributeName, std::list<const std::string*>& values) const
	{
		if (attributeName == "orientation")
		{
			static std::string kHorizontal = "horizontal";
			static std::string kVertical = "vertical";
	
			values.push_back (&kHorizontal);
			values.push_back (&kVertical);
			return true;
		}
		else if (attributeName == "resize-method")
		{
			static std::string kFirst = "first";
			static std::string kSecond = "second";
			static std::string kLast = "last";
			static std::string kAll = "all";
	
			values.push_back (&kFirst);
			values.push_back (&kSecond);
			values.push_back (&kLast);
			values.push_back (&kAll);
			return true;
		}
		return false;
	}

};
CSplitViewCreator __gCSplitViewCreator;

//-----------------------------------------------------------------------------
class CShadowViewContainerCreator : public IViewCreator
{
public:
	CShadowViewContainerCreator () { UIViewFactory::registerViewCreator (*this); }
	IdStringPtr getViewName () const { return "CShadowViewContainer"; }
	IdStringPtr getBaseViewName () const { return "CViewContainer"; }
	CView* create (const UIAttributes& attributes, IUIDescription* description) const 
	{
		return new CShadowViewContainer (CRect (0, 0, 200, 200));
	}

	bool apply (CView* view, const UIAttributes& attributes, IUIDescription* description) const
	{
		CShadowViewContainer* shadowView = dynamic_cast<CShadowViewContainer*> (view);
		if (!shadowView)
			return false;
		double d;
		if (attributes.getDoubleAttribute ("shadow-intensity", d))
			shadowView->setShadowIntensity ((float)d);
		int32_t i;
		if (attributes.getIntegerAttribute ("shadow-blur-size", i))
			shadowView->setShadowBlurSize (i);
		CPoint p;
		if (attributes.getPointAttribute("shadow-offset", p))
			shadowView->setShadowOffset (p);
		return true;
	}
	bool getAttributeNames (std::list<std::string>& attributeNames) const
	{
		attributeNames.push_back ("shadow-intensity");
		attributeNames.push_back ("shadow-offset");
		attributeNames.push_back ("shadow-blur-size");
		return true;
	}
	AttrType getAttributeType (const std::string& attributeName) const
	{
		if (attributeName == "shadow-intensity") return kFloatType;
		if (attributeName == "shadow-offset") return kPointType;
		if (attributeName == "shadow-blur-size") return kIntegerType;
		return kUnknownType;
	}
	bool getAttributeValue (CView* view, const std::string& attributeName, std::string& stringValue, IUIDescription* desc) const
	{
		CShadowViewContainer* shadowView = dynamic_cast<CShadowViewContainer*> (view);
		if (!shadowView)
			return false;
		if (attributeName == "shadow-intensity")
		{
			std::stringstream str;
			str << shadowView->getShadowIntensity ();
			stringValue = str.str ().c_str ();
			return true;
		}
		else if (attributeName == "shadow-blur-size")
		{
			std::stringstream str;
			str << shadowView->getShadowBlurSize ();
			stringValue = str.str ().c_str ();
			return true;
		}
		else if (attributeName == "shadow-offset")
		{
			pointToString (shadowView->getShadowOffset (), stringValue);
			return true;
		}
		return false;
	}
};
CShadowViewContainerCreator __gCShadowViewContainerCreator;

//-----------------------------------------------------------------------------
class CGradientViewCreator : public IViewCreator
{
public:
	CGradientViewCreator () { UIViewFactory::registerViewCreator (*this); }
	IdStringPtr getViewName () const { return "CGradientView"; }
	IdStringPtr getBaseViewName () const { return "CView"; }
	CView* create (const UIAttributes& attributes, IUIDescription* description) const { return new CGradientView (CRect (0, 0, 100, 100)); }
	bool apply (CView* view, const UIAttributes& attributes, IUIDescription* description) const
	{
		CGradientView* gv = dynamic_cast<CGradientView*> (view);
		if (gv == 0)
			return false;
		CColor color;
		const std::string* attr = attributes.getAttributeValue ("frame-color");
		if (attr)
		{
			if (description->getColor (attr->c_str (), color))
			{
				rememberAttributeValueString (view, "frame-color", *attr);
				gv->setFrameColor (color);
			}
		}
		attr = attributes.getAttributeValue ("gradient-start-color");
		if (attr)
		{
			if (description->getColor (attr->c_str (), color))
			{
				rememberAttributeValueString (view, "gradient-start-color", *attr);
				gv->setGradientStartColor (color);
			}
		}
		attr = attributes.getAttributeValue ("gradient-end-color");
		if (attr)
		{
			if (description->getColor (attr->c_str (), color))
			{
				rememberAttributeValueString (view, "gradient-end-color", *attr);
				gv->setGradientEndColor (color);
			}
		}
		double d;
		if (attributes.getDoubleAttribute ("gradient-angle", d))
		{
			gv->setGradientAngle (d);
		}
		if (attributes.getDoubleAttribute ("gradient-start-color-offset", d))
		{
			gv->setGradientStartColorOffset (d);
		}
		if (attributes.getDoubleAttribute ("gradient-end-color-offset", d))
		{
			gv->setGradientEndColorOffset (d);
		}
		if (attributes.getDoubleAttribute ("round-rect-radius", d))
		{
			gv->setRoundRectRadius (d);
		}
		if (attributes.getDoubleAttribute ("frame-width", d))
		{
			gv->setFrameWidth (d);
		}
		bool b;
		if (attributes.getBooleanAttribute ("draw-antialiased", b))
		{
			gv->setDrawAntialiased (b);
		}
		return true;
	}
	bool getAttributeNames (std::list<std::string>& attributeNames) const
	{
		attributeNames.push_back ("frame-color");
		attributeNames.push_back ("gradient-start-color");
		attributeNames.push_back ("gradient-end-color");
		attributeNames.push_back ("gradient-angle");
		attributeNames.push_back ("gradient-start-color-offset");
		attributeNames.push_back ("gradient-end-color-offset");
		attributeNames.push_back ("round-rect-radius");
		attributeNames.push_back ("frame-width");
		attributeNames.push_back ("draw-antialiased");
		return true;
	}
	AttrType getAttributeType (const std::string& attributeName) const
	{
		if (attributeName == "frame-color") return kColorType;
		if (attributeName == "gradient-start-color") return kColorType;
		if (attributeName == "gradient-end-color") return kColorType;
		if (attributeName == "gradient-angle") return kFloatType;
		if (attributeName == "gradient-start-color-offset") return kFloatType;
		if (attributeName == "gradient-end-color-offset") return kFloatType;
		if (attributeName == "round-rect-radius") return kFloatType;
		if (attributeName == "frame-width") return kFloatType;
		if (attributeName == "draw-antialiased") return kBooleanType;
		return kUnknownType;
	}
	bool getAttributeValue (CView* view, const std::string& attributeName, std::string& stringValue, IUIDescription* desc) const
	{
		CGradientView* gv = dynamic_cast<CGradientView*> (view);
		if (gv == 0)
			return false;
		if (attributeName == "frame-color")
		{
			if (!getRememberedAttributeValueString (view, "frame-color", stringValue))
				colorToString (gv->getFrameColor (), stringValue, desc);
			return true;
		}
		if (attributeName == "gradient-start-color")
		{
			if (!getRememberedAttributeValueString (view, "gradient-start-color", stringValue))
				colorToString (gv->getGradientStartColor (), stringValue, desc);
			return true;
		}
		if (attributeName == "gradient-end-color")
		{
			if (!getRememberedAttributeValueString (view, "gradient-end-color", stringValue))
				colorToString (gv->getGradientEndColor (), stringValue, desc);
			return true;
		}
		if (attributeName == "gradient-angle")
		{
			std::stringstream str;
			str << gv->getGradientAngle ();
			stringValue = str.str ().c_str ();
			return true;
		}
		if (attributeName == "gradient-start-color-offset")
		{
			std::stringstream str;
			str << gv->getGradientStartColorOffset ();
			stringValue = str.str ().c_str ();
			return true;
		}
		if (attributeName == "gradient-end-color-offset")
		{
			std::stringstream str;
			str << gv->getGradientEndColorOffset ();
			stringValue = str.str ().c_str ();
			return true;
		}
		if (attributeName == "round-rect-radius")
		{
			std::stringstream str;
			str << gv->getRoundRectRadius ();
			stringValue = str.str ().c_str ();
			return true;
		}
		if (attributeName == "frame-width")
		{
			std::stringstream str;
			str << gv->getFrameWidth ();
			stringValue = str.str ().c_str ();
			return true;
		}
		if (attributeName == "draw-antialiased")
		{
			stringValue = gv->getDrawAntialised () ? "true" : "false";
			return true;
		}
		return false;
	}

};
CGradientViewCreator __gCGradientViewCreator;

//-----------------------------------------------------------------------------
class CXYPadCreator : public IViewCreator
{
public:
	CXYPadCreator () { UIViewFactory::registerViewCreator (*this); }
	IdStringPtr getViewName () const { return "CXYPad"; }
	IdStringPtr getBaseViewName () const { return "CParamDisplay"; }
	CView* create (const UIAttributes& attributes, IUIDescription* description) const { return new CXYPad (CRect (0, 0, 100, 20)); }
	bool apply (CView* view, const UIAttributes& attributes, IUIDescription* description) const
	{
		return true;
	}
	bool getAttributeNames (std::list<std::string>& attributeNames) const
	{
		return true;
	}
	AttrType getAttributeType (const std::string& attributeName) const
	{
		return kUnknownType;
	}
	bool getAttributeValue (CView* view, const std::string& attributeName, std::string& stringValue, IUIDescription* desc) const
	{
		return false;
	}
};
CXYPadCreator __gCXYPadCreator;


}} // namespace

/**
@endcond ignore
*/
