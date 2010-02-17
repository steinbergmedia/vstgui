//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework not only for VST plugins : 
//
// Version 4.0
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2010, Steinberg Media Technologies, All Rights Reserved
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
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A  PARTICULAR PURPOSE ARE DISCLAIMED. 
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
	const char* getViewName () const { return "CView"; }
	const char* getBaseViewName () const { return 0; }
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

#include "viewfactory.h"
#include "../vstgui.h"
#include <sstream>

#if WINDOWS
#define strtof	(float)strtod
#endif

namespace VSTGUI {

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
	const char* bitmapName = desc->lookupBitmapName (bitmap);
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
	const char* colorName = desc ? desc->lookupColorName (color) : 0;
	if (colorName)
		string = colorName;
	else
	{
		unsigned char red = color.red;
		unsigned char green = color.green;
		unsigned char blue = color.blue;
		unsigned char alpha = color.alpha;
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
	CViewCreator () { ViewFactory::registerViewCreator (*this); }
	const char* getViewName () const { return "CView"; }
	const char* getBaseViewName () const { return 0; }
	CView* create (const UIAttributes& attributes, IUIDescription* description) const { return new CView (CRect (0, 0, 0, 0)); }
	bool apply (CView* view, const UIAttributes& attributes, IUIDescription* description) const
	{
		const std::string* originAttr = attributes.getAttributeValue ("origin");
		const std::string* sizeAttr = attributes.getAttributeValue ("size");
		const std::string* transparentAttr = attributes.getAttributeValue ("transparent");
		const std::string* bitmapAttr = attributes.getAttributeValue ("bitmap");
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
		
		if (transparentAttr)
			view->setTransparency (*transparentAttr == "true");

		if (autosizeAttr)
		{
			long autosize = kAutosizeNone;
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
				view->setAttribute (kCViewTooltipAttribute, tooltipAttr->size ()+1, tooltipAttr->c_str ());
			else
				view->removeAttribute (kCViewTooltipAttribute);
		}
		if (customViewAttr)
			view->setAttribute ('uicv', customViewAttr->size ()+1, customViewAttr->c_str ());

		if (subControllerAttr)
			view->setAttribute ('uisc', subControllerAttr->size ()+1, subControllerAttr->c_str ());

		return true;
	}
	bool getAttributeNames (std::list<std::string>& attributeNames) const
	{
		attributeNames.push_back ("origin");
		attributeNames.push_back ("size");
		attributeNames.push_back ("transparent");
		attributeNames.push_back ("bitmap");
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
		else if (attributeName == "bitmap") return kBitmapType;
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
		else if (attributeName == "bitmap")
		{
			CBitmap* bitmap = view->getBackground ();
			if (bitmap)
				bitmapToString (bitmap, stringValue, desc);
			else
				stringValue = "";
			return true;
		}
		else if (attributeName == "autosize")
		{
			std::stringstream stream;
			long autosize = view->getAutosizeFlags ();
			if (autosize == 0)
				return false;
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
			long tooltipSize = 0;
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
			long customViewNameSize = 0;
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
			long subControllerNameSize = 0;
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
static unsigned int DJBHash (const std::string& str)
{
   unsigned int hash = 5381;
   for (std::size_t i = 0; i < str.length (); i++)
   {
      hash = ((hash << 5) + hash) + str[i];
   }
   return hash;
}

//-----------------------------------------------------------------------------
void rememberAttributeValueString (CView* view, const char* attrName, const std::string& value)
{
	#if VSTGUI_LIVE_EDITING
	unsigned int hash = DJBHash (attrName);
	view->setAttribute (hash, value.size () + 1, value.c_str ());
	#endif
}

//-----------------------------------------------------------------------------
bool getRememberedAttributeValueString (CView* view, const char* attrName, std::string& value)
{
	bool result = false;
	#if VSTGUI_LIVE_EDITING
	unsigned int hash = DJBHash (attrName);
	long attrSize = 0;
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
	CViewContainerCreator () { ViewFactory::registerViewCreator (*this); }
	const char* getViewName () const { return "CViewContainer"; }
	const char* getBaseViewName () const { return "CView"; }
	CView* create (const UIAttributes& attributes, IUIDescription* description) const { return new CViewContainer (CRect (0, 0, 100, 100), 0); }
	bool apply (CView* view, const UIAttributes& attributes, IUIDescription* description) const
	{
		CViewContainer* viewContainer = dynamic_cast<CViewContainer*> (view);
		if (viewContainer == 0)
			return false;
		const std::string* backColorAttr = attributes.getAttributeValue ("background-color");
		if (backColorAttr)
		{
			CColor backColor;
			if (description->getColor (backColorAttr->c_str (), backColor))
			{
				rememberAttributeValueString (view, "background-color", *backColorAttr);
				viewContainer->setBackgroundColor (backColor);
			}
		}
		return true;
	}
	bool getAttributeNames (std::list<std::string>& attributeNames) const
	{
		attributeNames.push_back ("background-color");
		return true;
	}
	AttrType getAttributeType (const std::string& attributeName) const
	{
		if (attributeName == "background-color") return kColorType;
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
		return false;
	}

};
CViewContainerCreator __CViewContainerCreator;

//-----------------------------------------------------------------------------
class CScrollViewCreator : public IViewCreator
{
public:
	CScrollViewCreator () { ViewFactory::registerViewCreator (*this); }
	const char* getViewName () const { return "CScrollView"; }
	const char* getBaseViewName () const { return "CViewContainer"; }
	CView* create (const UIAttributes& attributes, IUIDescription* description) const { return new CScrollView (CRect (0, 0, 100, 100), CRect (0, 0, 200, 200), 0, CScrollView::kHorizontalScrollbar|CScrollView::kVerticalScrollbar); }
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
		long style = scrollView->getStyle ();
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
		attr = attributes.getAttributeValue ("bordered");
		if (attr)
		{
			if (*attr == "true")
				style &= ~CScrollView::kDontDrawFrame;
			else
				style |= CScrollView::kDontDrawFrame;
		}
		scrollView->setStyle (style);
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
		attributeNames.push_back ("scrollbar-width");
		attributeNames.push_back ("bordered");
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
		if (attributeName == "bordered") return kBooleanType;
		if (attributeName == "scrollbar-width") return kIntegerType;
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
		if (attributeName == "bordered")
		{
			stringValue = sc->getStyle () & CScrollView::kDontDrawFrame ? "false" : "true";
			return true;
		}
		return false;
	}

};
CScrollViewCreator __CScrollViewCreator;

//-----------------------------------------------------------------------------
class CControlCreator : public IViewCreator
{
public:
	CControlCreator () { ViewFactory::registerViewCreator (*this); }
	const char* getViewName () const { return "CControl"; }
	const char* getBaseViewName () const { return "CView"; }
	CView* create (const UIAttributes& attributes, IUIDescription* description) const { return 0; }
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
				long tag = description->getTagForName (controlTagAttr->c_str ());
				if (tag != -1)
				{
					control->setTag (tag);
					control->setListener (description->getControlListener (controlTagAttr->c_str ()));
				}
				else
				{
					char* endPtr = 0;
					tag = strtol (controlTagAttr->c_str (), &endPtr, 10);
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
				const char* controlTag = desc->lookupControlTagName (control->getTag ());
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
	COnOffButtonCreator () { ViewFactory::registerViewCreator (*this); }
	const char* getViewName () const { return "COnOffButton"; }
	const char* getBaseViewName () const { return "CControl"; }
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
	CCheckBoxCreator () { ViewFactory::registerViewCreator (*this); }
	const char* getViewName () const { return "CCheckBox"; }
	const char* getBaseViewName () const { return "CControl"; }
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
		long style = checkbox->getStyle ();
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
			const char* fontName = desc->lookupFontName (checkbox->getFont ());
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
	CParamDisplayCreator () { ViewFactory::registerViewCreator (*this); }
	const char* getViewName () const { return "CParamDisplay"; }
	const char* getBaseViewName () const { return "CControl"; }
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
		const std::string* textAlignmentAttr = attributes.getAttributeValue ("text-alignment");
		const std::string* textInsetAttr = attributes.getAttributeValue ("text-inset");

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
		long style = display->getStyle ();
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
		display->setStyle (style);
		return true;
	}
	bool getAttributeNames (std::list<std::string>& attributeNames) const
	{
		attributeNames.push_back ("font");
		attributeNames.push_back ("font-color");
		attributeNames.push_back ("back-color");
		attributeNames.push_back ("frame-color");
		attributeNames.push_back ("shadow-color");
		attributeNames.push_back ("font-antialias");
		attributeNames.push_back ("style-3D-in");
		attributeNames.push_back ("style-3D-out");
		attributeNames.push_back ("style-no-frame");
		attributeNames.push_back ("style-no-text");
		attributeNames.push_back ("style-no-draw");
		attributeNames.push_back ("style-shadow-text");
		attributeNames.push_back ("text-alignment");
		attributeNames.push_back ("text-inset");
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
		else if (attributeName == "text-alignment") return kStringType;
		else if (attributeName == "text-inset") return kPointType;
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
			const char* fontName = desc->lookupFontName (pd->getFont ());
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
		return false;
	}

};
CParamDisplayCreator __gCParamDisplayCreator;

//-----------------------------------------------------------------------------
class COptionMenuCreator : public IViewCreator
{
public:
	COptionMenuCreator () { ViewFactory::registerViewCreator (*this); }
	const char* getViewName () const { return "COptionMenu"; }
	const char* getBaseViewName () const { return "CParamDisplay"; }
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
	CTextLabelCreator () { ViewFactory::registerViewCreator (*this); }
	const char* getViewName () const { return "CTextLabel"; }
	const char* getBaseViewName () const { return "CParamDisplay"; }
	CView* create (const UIAttributes& attributes, IUIDescription* description) const { return new CTextLabel (CRect (0, 0, 0, 0)); }
	bool apply (CView* view, const UIAttributes& attributes, IUIDescription* description) const
	{
		CTextLabel* label = dynamic_cast<CTextLabel*> (view);
		if (!label)
			return false;

		const std::string* titleAttr = attributes.getAttributeValue ("title");
		if (titleAttr)
			label->setText (titleAttr->c_str ());

		return true;
	}
	bool getAttributeNames (std::list<std::string>& attributeNames) const
	{
		attributeNames.push_back ("title");
		return true;
	}
	AttrType getAttributeType (const std::string& attributeName) const
	{
		if (attributeName == "title") return kStringType;
		return kUnknownType;
	}
	bool getAttributeValue (CView* view, const std::string& attributeName, std::string& stringValue, IUIDescription* desc) const
	{
		CTextLabel* label = dynamic_cast<CTextLabel*> (view);
		if (!label)
			return false;
		if (attributeName == "title")
		{
			const char* title = label->getText ();
			stringValue = title ? title : "";
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
	CTextEditCreator () { ViewFactory::registerViewCreator (*this); }
	const char* getViewName () const { return "CTextEdit"; }
	const char* getBaseViewName () const { return "CParamDisplay"; }
	CView* create (const UIAttributes& attributes, IUIDescription* description) const { return new CTextEdit (CRect (0, 0, 0, 0), 0, -1); }
	bool apply (CView* view, const UIAttributes& attributes, IUIDescription* description) const
	{
		CTextEdit* label = dynamic_cast<CTextEdit*> (view);
		if (!label)
			return false;

		const std::string* titleAttr = attributes.getAttributeValue ("title");
		if (titleAttr)
			label->setText (titleAttr->c_str ());

		return true;
	}
	bool getAttributeNames (std::list<std::string>& attributeNames) const
	{
		attributeNames.push_back ("title");
		return true;
	}
	AttrType getAttributeType (const std::string& attributeName) const
	{
		if (attributeName == "title") return kStringType;
		return kUnknownType;
	}
	bool getAttributeValue (CView* view, const std::string& attributeName, std::string& stringValue, IUIDescription* desc) const
	{
		CTextEdit* label = dynamic_cast<CTextEdit*> (view);
		if (!label)
			return false;
		if (attributeName == "title")
		{
			const char* title = label->getText ();
			stringValue = title ? title : "";
			return true;
		}
		return false;
	}

};
CTextEditCreator __gCTextEditCreator;

//-----------------------------------------------------------------------------
class CKnobCreator : public IViewCreator
{
public:
	CKnobCreator () { ViewFactory::registerViewCreator (*this); }
	const char* getViewName () const { return "CKnob"; }
	const char* getBaseViewName () const { return "CControl"; }
	CView* create (const UIAttributes& attributes, IUIDescription* description) const { return new CKnob (CRect (0, 0, 0, 0), 0, -1, 0, 0); }
	bool apply (CView* view, const UIAttributes& attributes, IUIDescription* description) const
	{
		CKnob* knob = dynamic_cast<CKnob*> (view);
		if (!knob)
			return false;

		const std::string* angleStartAttr = attributes.getAttributeValue ("angle-start");
		const std::string* angleRangeAttr = attributes.getAttributeValue ("angle-range");
		const std::string* insetValueAttr = attributes.getAttributeValue ("value-inset");
		const std::string* zoomFactorAttr = attributes.getAttributeValue ("zoom-factor");
		const std::string* handleShadowColorAttr = attributes.getAttributeValue ("handle-shadow-color");
		const std::string* handleColorAttr = attributes.getAttributeValue ("handle-color");
		const std::string* handleBitmapAttr = attributes.getAttributeValue ("handle-bitmap");

		float fvalue = 0.f;
		long lvalue = 0;
		CColor color;
		if (angleStartAttr)
		{
			fvalue = strtof (angleStartAttr->c_str (), 0);
			knob->setStartAngle (fvalue);
		}
		if (angleRangeAttr)
		{
			fvalue = strtof (angleRangeAttr->c_str (), 0);
			knob->setRangeAngle (fvalue);
		}
		if (insetValueAttr)
		{
			lvalue = strtol (insetValueAttr->c_str (), 0, 10);
			knob->setInsetValue (lvalue);
		}
		if (zoomFactorAttr)
		{
			fvalue = strtof (zoomFactorAttr->c_str (), 0);
			knob->setZoomFactor (fvalue);
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
		if (handleBitmapAttr)
		{
			CBitmap* bitmap = description->getBitmap (handleBitmapAttr->c_str ());
			if (bitmap)
				knob->setHandleBitmap (bitmap);
		}
		return true;
	}
	bool getAttributeNames (std::list<std::string>& attributeNames) const
	{
		attributeNames.push_back ("angle-start");
		attributeNames.push_back ("angle-range");
		attributeNames.push_back ("value-inset");
		attributeNames.push_back ("zoom-factor");
		attributeNames.push_back ("handle-shadow-color");
		attributeNames.push_back ("handle-color");
		attributeNames.push_back ("handle-bitmap");
		return true;
	}
	AttrType getAttributeType (const std::string& attributeName) const
	{
		if (attributeName == "angle-start") return kFloatType;
		else if (attributeName == "angle-range") return kFloatType;
		else if (attributeName == "value-inset") return kIntegerType;
		else if (attributeName == "zoom-factor") return kFloatType;
		else if (attributeName == "handle-shadow-color") return kColorType;
		else if (attributeName == "handle-color") return kColorType;
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
			stream << knob->getStartAngle ();
			stringValue = stream.str ();
			return true;
		}
		else if (attributeName == "angle-range")
		{
			std::stringstream stream;
			stream << knob->getRangeAngle ();
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
		else if (attributeName == "zoom-factor")
		{
			std::stringstream stream;
			stream << knob->getZoomFactor ();
			stringValue = stream.str ();
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
			long value = strtol (attr->c_str (), 0, 10);
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
	CAnimKnobCreator () { ViewFactory::registerViewCreator (*this); }
	const char* getViewName () const { return "CAnimKnob"; }
	const char* getBaseViewName () const { return "CKnob"; }
	CView* create (const UIAttributes& attributes, IUIDescription* description) const { return new CAnimKnob (CRect (0, 0, 0, 0), 0, -1, 0); }
};
CAnimKnobCreator __gCAnimKnobCreator;

//-----------------------------------------------------------------------------
class CVerticalSwitchCreator : public IMultiBitmapControlCreator
{
public:
	CVerticalSwitchCreator () { ViewFactory::registerViewCreator (*this); }
	const char* getViewName () const { return "CVerticalSwitch"; }
	const char* getBaseViewName () const { return "CControl"; }
	CView* create (const UIAttributes& attributes, IUIDescription* description) const { return new CVerticalSwitch (CRect (0, 0, 0, 0), 0, -1, 0); }
};
CVerticalSwitchCreator __gCVerticalSwitchCreator;

//-----------------------------------------------------------------------------
class CHorizontalSwitchCreator : public IMultiBitmapControlCreator
{
public:
	CHorizontalSwitchCreator () { ViewFactory::registerViewCreator (*this); }
	const char* getViewName () const { return "CHorizontalSwitch"; }
	const char* getBaseViewName () const { return "CControl"; }
	CView* create (const UIAttributes& attributes, IUIDescription* description) const { return new CHorizontalSwitch (CRect (0, 0, 0, 0), 0, -1, 0); }
};
CHorizontalSwitchCreator __gCHorizontalSwitchCreator;

//-----------------------------------------------------------------------------
class CRockerSwitchCreator : public IMultiBitmapControlCreator
{
public:
	CRockerSwitchCreator () { ViewFactory::registerViewCreator (*this); }
	const char* getViewName () const { return "CRockerSwitch"; }
	const char* getBaseViewName () const { return "CControl"; }
	CView* create (const UIAttributes& attributes, IUIDescription* description) const { return new CRockerSwitch (CRect (0, 0, 0, 0), 0, -1, 0); }
};
CRockerSwitchCreator __gCRockerSwitchCreator;

//-----------------------------------------------------------------------------
class CMovieBitmapCreator : public IMultiBitmapControlCreator
{
public:
	CMovieBitmapCreator () { ViewFactory::registerViewCreator (*this); }
	const char* getViewName () const { return "CMovieBitmap"; }
	const char* getBaseViewName () const { return "CControl"; }
	CView* create (const UIAttributes& attributes, IUIDescription* description) const { return new CMovieBitmap (CRect (0, 0, 0, 0), 0, -1, 0); }
};
CMovieBitmapCreator __gCMovieBitmapCreator;

//-----------------------------------------------------------------------------
class CMovieButtonCreator : public IMultiBitmapControlCreator
{
public:
	CMovieButtonCreator () { ViewFactory::registerViewCreator (*this); }
	const char* getViewName () const { return "CMovieButton"; }
	const char* getBaseViewName () const { return "CControl"; }
	CView* create (const UIAttributes& attributes, IUIDescription* description) const { return new CMovieButton (CRect (0, 0, 0, 0), 0, -1, 0); }
};
CMovieButtonCreator __gCMovieButtonCreator;

//-----------------------------------------------------------------------------
class CKickButtonCreator : public IMultiBitmapControlCreator
{
public:
	CKickButtonCreator () { ViewFactory::registerViewCreator (*this); }
	const char* getViewName () const { return "CKickButton"; }
	const char* getBaseViewName () const { return "CControl"; }
	CView* create (const UIAttributes& attributes, IUIDescription* description) const { return new CKickButton (CRect (0, 0, 0, 0), 0, -1, 0); }
};
CKickButtonCreator __gCKickButtonCreator;

//-----------------------------------------------------------------------------
class CSliderCreator : public IViewCreator
{
public:
	CSliderCreator () { ViewFactory::registerViewCreator (*this); }
	const char* getViewName () const { return "CSlider"; }
	const char* getBaseViewName () const { return "CControl"; }
	CView* create (const UIAttributes& attributes, IUIDescription* description) const { return new CSlider (CRect (0, 0, 0, 0), 0, -1, 0, 0, 0, 0); }
	bool apply (CView* view, const UIAttributes& attributes, IUIDescription* description) const
	{
		CSlider* slider = dynamic_cast<CSlider*> (view);
		if (!slider)
			return false;

		const std::string* transparentHandleAttr = attributes.getAttributeValue ("transparent-handle");
		const std::string* freeClickAttr = attributes.getAttributeValue ("free-click");
		const std::string* handleBitmapAttr = attributes.getAttributeValue ("handle-bitmap");
		const std::string* handleOffsetAttr = attributes.getAttributeValue ("handle-offset");
		const std::string* bitmapOffsetAttr = attributes.getAttributeValue ("bitmap-offset");
		const std::string* zoomFactorAttr = attributes.getAttributeValue ("zoom-factor");
		const std::string* orientationAttr = attributes.getAttributeValue ("orientation");
		const std::string* reverseOrientationAttr = attributes.getAttributeValue ("reverse-orientation");

		CPoint p;
		if (transparentHandleAttr)
			slider->setDrawTransparentHandle (*transparentHandleAttr == "true");
		if (freeClickAttr)
			slider->setFreeClick (*freeClickAttr == "true");
		if (handleBitmapAttr)
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
			long style = slider->getStyle ();
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
			long style = slider->getStyle ();
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
		return true;
	}
	bool getAttributeNames (std::list<std::string>& attributeNames) const
	{
		attributeNames.push_back ("transparent-handle");
		attributeNames.push_back ("free-click");
		attributeNames.push_back ("handle-bitmap");
		attributeNames.push_back ("handle-offset");
		attributeNames.push_back ("bitmap-offset");
		attributeNames.push_back ("zoom-factor");
		attributeNames.push_back ("orientation");
		attributeNames.push_back ("reverse-orientation");
		return true;
	}
	AttrType getAttributeType (const std::string& attributeName) const
	{
		if (attributeName == "transparent-handle") return kBooleanType;
		if (attributeName == "free-click") return kBooleanType;
		if (attributeName == "handle-bitmap") return kBitmapType;
		if (attributeName == "handle-offset") return kPointType;
		if (attributeName == "bitmap-offset") return kPointType;
		if (attributeName == "zoom-factor") return kFloatType;
		if (attributeName == "orientation") return kStringType;
		if (attributeName == "reverse-orientation") return kBooleanType;
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
		else if (attributeName == "free-click")
		{
			stringValue = slider->getFreeClick () ? "true" : "false";
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
			long style = slider->getStyle ();
			stringValue = "false";
			if (((style & kVertical) && (style & kTop)) || ((style & kHorizontal) && (style & kRight)))
				stringValue = "true";
			else
				stringValue = "false";
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
	CVuMeterCreator () { ViewFactory::registerViewCreator (*this); }
	const char* getViewName () const { return "CVuMeter"; }
	const char* getBaseViewName () const { return "CControl"; }
	CView* create (const UIAttributes& attributes, IUIDescription* description) const { return new CVuMeter (CRect (0, 0, 0, 0), 0, 0, 100); }
	bool apply (CView* view, const UIAttributes& attributes, IUIDescription* description) const
	{
		CVuMeter* vuMeter = dynamic_cast<CVuMeter*> (view);
		if (!vuMeter)
			return false;

		const std::string* attr = attributes.getAttributeValue ("off-bitmap");
		if (attr)
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
			long numLed = strtol (attr->c_str (), 0, 10);
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
		if (attributeName == "orientation") return kStringType;
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

};
CVuMeterCreator __gCVuMeterCreator;

} // namespace

/**
@endcond ignore
*/