/**
@page uidescription_attributes Creating User Interfaces via XML

@section vstguiandxml VSTGUI and XML
It is now possible to create VSTGUI based interfaces via a XML description.

- @ref creatingbycode @n
- @ref customviews @n
- @ref examplexml @n
- @ref defbitmaps @n
- @ref deffonts @n
- @ref defcolors @n
- @ref deftags @n
- @ref deftemplates @n
- @ref viewclasses @n

@section creatingbycode Creating a view
You need to write a XML text file like the one in the example shown later on.
On Mac OS X this xml file must be placed into the Resources folder of the bundle and on Windows it must be declared in the .rc file.
To use the xml file to create views you have to write this code :
@code
UIDescription description ("myview.xml");
if (description.parse ())
{
  CView* view = description.createView ("MyEditor", 0);
}
@endcode

If view is non-null it was successfully created and you can add it to your CFrame object.

@section customviews Creating custom views
If you want to create your own custom views, you have two options:
-# Create view factory methods for your custom views (look into viewcreator.cpp how this is done for the built in views)
-# Inherit a class from VSTGUI::IController and provide the view in the VSTGUI::IController::createView method. An instance of this class must be passed as second argument to the createView method of UIDescription.


@section examplexml Example XML file

First let us see a simple example of XML text describing a VSTGUI view hierarchy:
@verbatim
<?xml version="1.0" encoding="UTF-8"?>
<vstgui-ui-description version="1">
  <bitmaps>
    <bitmap name="background" path="background.png"/>
    <bitmap name="slider-handle" path="slider-handle.png"/>
    <bitmap name="slider-background" path="slider-background.png"/>
  </bitmaps>

  <fonts>
    <font name="labelfont" font-name="Arial" size="11" bold="false" italic="false"/>
  </fonts>

  <colors>
    <color name="labelcolor" red="255" green="0" blue="255" alpha="255"/>
  </colors>

  <control-tags>
    <control-tag name="tag 1" tag="0"/>
    <control-tag name="tag 2" tag="1"/>
  </control-tags>

  <template name="MyEditor" size="500, 320" background-color="#000000DD" minSize="500, 320" maxSize="1000, 320" autosize="left right">
    <view class="CViewContainer" origin="10, 10" size="480, 90" background-color="#FFFFFF22" autosize="left right">
      <view class="CTextLabel" title="Test Label" origin="10, 10" size="80,20" transparent="true" font-color="labelcolor" font="labelfont"/>
      <view class="CSlider" control-tag="tag 1" origin="110, 10" size="260,20" handle-offset="3,3" bitmap="slider-background" handle-bitmap="slider-handle" autosize="left right"/>
      <view class="CTextEdit" control-tag="tag 2" origin="390, 10" size="80,20" back-color="slider-back" frame-color="slider-frame" font-color="labelcolor" font="labelfont" autosize="right"/>
    </view>
  </template>
</vstgui-ui-description>
@endverbatim

@section defbitmaps Defining Bitmaps
Any bitmap you want to use with your views must be declared inside the \b bitmaps tag. Recognized attributes for the \b bitmap tag are:
- \b name
<br/>you refer to this name later on when you want to use this bitmap
- \b path
<br/>the path to the bitmap (On Mac OS X this is the path inside the Resource directory of the bundle and on Windows this is the name used in the .rc file)
.
Example:
@verbatim
<bitmaps>
  <bitmap name="background" path="background.png"/>
</bitmaps>
@endverbatim

@section deffonts Defining Fonts
Any font you want to use with your views must be declared inside the \b fonts tag. Recognized attributes for the \b font tag are:
- \b name
<br/>you refer to this name later on when you want to use this font
- \b font-name
<br/>the system font name
- \b size
<br/>size of the font
- \b bold
<br/>true or false
- \b italic
<br/>true or false
- \b underline
<br/>true or false
.
Example:
@verbatim
<fonts>
  <font name="labelfont" font-name="Arial" size="11" bold="false" italic="false"/>
</fonts>
@endverbatim

@section defcolors Defining Colors
You can define global colors within the \b colors tag. Recognized attributes for the \b color tag are:
- \b name
<br/>you refer to this name later on when you want to use this color
- \b red
<br/>the red value of this color in the range from 0 to 255
- \b green
<br/>the green value of this color in the range from 0 to 255
- \b blue
<br/>the blue value of this color in the range from 0 to 255
- \b alpha
<br/>the alpha value of this color in the range from 0 to 255
- \b rgb
<br/>the red, green and blue values in hex notation known from HTML and CSS: #0055BB (the alpha value of this color is always 255, and it overrides any previous attribute)
- \b rgba
<br/>the red, green, blue and alpha values in hex notation known from HTML and CSS: #005566FF (any previous attribute will be ignored)
.
Example:
@verbatim
<colors>
  <color name="labelcolor" rgba="#005566FF"/>
  <color name="labelcolor2" rgb="#005566"/>
  <color name="labelcolor3" red="0" green="85" blue="102" alpha="255"/>
  <color name="labelcolor4" green="85" blue="102"/>
  <!-- by the way, these colors have all the same rgba values -->
</colors>
@endverbatim
Colors can also be declared within the \b view tag for any color tag with one of the two hex notations. The following colors are predefined:
black, white, grey, red, green, blue, yellow, cyan, magenta.


@section deftags Defining Tags
VSTGUI controls are identified by tags. In the \b control-tags tag you map control tags to names. Recognized attributes in the \b control-tag tag are:
- \b name
<br/>you refer to this name later on when you want to use this control tag
- \b tag
<br/>an integer tag or a tag defined like 'abcd'
.
Example:
@verbatim
<control-tags>
  <control-tag name="tag 1" tag="0"/>
  <control-tag name="tag 2" tag="'abcd'"/>
</control-tags>
@endverbatim

@section deftemplates Defining Templates
Templates are the main views in XML. You can have more than one.
Per default the \b template tag will create a CViewContainer view, but you can use the \b class attribute to create any view class you want.
(If the template should have subviews, the class must be an inherited class from CViewContainer like CScrollView)<br/>
See the next section for recognized attributes.

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
Declaration: <view class="CView" />

Attributes:
- \b origin [Point]
- \b size [Point]
- \b transparent [true/false]
- \b bitmap [bitmap name]
- \b autosize [combination of <i>left</i>, <i>top</i>, <i>right</i>, <i>bottom</i>, <i>row</i>, or <i>column</i> see VSTGUI::CViewAutosizing]
- \b tooltip [tooltip text]

@section cviewcontainer CViewContainer
Declaration: <view class="CViewContainer" />

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
Declaration: <view class="COnOffButton" />

Inherites attributes from @ref ccontrol @n

@section cparamdisplay CParamDisplay
Declaration: <view class="CParamDisplay" />

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
Declaration: <view class="CTextLabel" />

Inherites attributes from @ref cparamdisplay @n

Attributes:
- \b title [string]

@section ctextedit CTextEdit
Declaration: <view class="CTextEdit" />

Inherites attributes from @ref cparamdisplay @n

Attributes:
- \b title [string]

@section cknob CKnob
Declaration: <view class="CKnob" />

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
Declaration: <view class="CAnimKnob" />

Inherites attributes from @ref cknob @n

Attributes:
- \b height-of-one-image [int]

@section cverticalswitch CVerticalSwitch
Declaration: <view class="CVerticalSwitch" />

Inherites attributes from @ref ccontrol @n

Attributes:
- \b height-of-one-image [int]

@section chorizontalswitch CHorizontalSwitch
Declaration: <view class="CHorizontalSwitch" />

Inherites attributes from @ref ccontrol @n

Attributes:
- \b height-of-one-image [int]

@section crockerswitch CRockerSwitch
Declaration: <view class="CRockerSwitch" />

Inherites attributes from @ref ccontrol @n

Attributes:
- \b height-of-one-image [int]

@section cmoviebitmap CMovieBitmap
Declaration: <view class="CMovieBitmap" />

Inherites attributes from @ref ccontrol @n

Attributes:
- \b height-of-one-image [int]

@section cmoviebutton CMovieButton
Declaration: <view class="CMovieButton" />

Inherites attributes from @ref ccontrol @n

Attributes:
- \b height-of-one-image [int]

@section ckickbutton CKickButton
Declaration: <view class="CKickButton" />

Inherites attributes from @ref ccontrol @n

Attributes:
- \b height-of-one-image [int]

@section cslider CSlider
Declaration: <view class="CSlider" />

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

#include "viewfactory.h"

BEGIN_NAMESPACE_VSTGUI

//-----------------------------------------------------------------------------
static bool parseSize (const std::string& str, CPoint& point)
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
// CView
//-----------------------------------------------------------------------------
CView* CViewCreateFunction (const UIAttributes& attributes, IViewFactory* factory, UIDescription* description)
{
	return new CView (CRect (0, 0, 0, 0));
}

//-----------------------------------------------------------------------------
bool CViewApplyAttributesFunction (CView* view, const UIAttributes& attributes, IViewFactory* factory, UIDescription* description)
{
	const std::string* originAttr = attributes.getAttributeValue ("origin");
	const std::string* sizeAttr = attributes.getAttributeValue ("size");
	const std::string* transparentAttr = attributes.getAttributeValue ("transparent");
	const std::string* bitmapAttr = attributes.getAttributeValue ("bitmap");
	const std::string* autosizeAttr = attributes.getAttributeValue ("autosize");
	const std::string* tooltipAttr = attributes.getAttributeValue ("tooltip");
	
	CPoint p;
	CRect size;
	if (originAttr)
	{
		if (parseSize (*originAttr, p))
			size.setTopLeft (p);
	}
	if (sizeAttr)
	{
		if (parseSize (*sizeAttr, p))
			size.setSize (p);
	}
	if (bitmapAttr)
	{
		CBitmap* bitmap = description->getBitmap (bitmapAttr->c_str ());
		if (bitmap)
		{
			view->setBackground (bitmap);
			if (!sizeAttr)
			{
				size.setWidth (bitmap->getWidth ());
				size.setHeight (bitmap->getHeight ());
			}
		}
	}
	view->setViewSize (size, false);
	view->setMouseableArea (size);
	
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
		view->setAttribute (kCViewTooltipAttribute, tooltipAttr->size ()+1, tooltipAttr->c_str ());

	return true;
}

//-----------------------------------------------------------------------------
REGISTER_VIEW_CREATOR(CView, "CView", 0, CViewCreateFunction, CViewApplyAttributesFunction)

//-----------------------------------------------------------------------------
// CViewContainer
//-----------------------------------------------------------------------------
CView* CViewContainerCreateFunction (const UIAttributes& attributes, IViewFactory* factory, UIDescription* description)
{
	return new CViewContainer (CRect (0, 0, 0, 0), 0);
}

//-----------------------------------------------------------------------------
bool CViewContainerApplyAttributesFunction (CView* view, const UIAttributes& attributes, IViewFactory* factory, UIDescription* description)
{
	CViewContainer* viewContainer = dynamic_cast<CViewContainer*> (view);
	if (!viewContainer)
		return false;

	const std::string* backColorAttr = attributes.getAttributeValue ("background-color");
	if (backColorAttr)
	{
		CColor backColor;
		if (description->getColor (backColorAttr->c_str (), backColor))
			viewContainer->setBackgroundColor (backColor);
	}
	return true;
}

//-----------------------------------------------------------------------------
REGISTER_VIEW_CREATOR(CViewContainer, "CViewContainer", "CView", CViewContainerCreateFunction, CViewContainerApplyAttributesFunction)

//-----------------------------------------------------------------------------
// CControl
//-----------------------------------------------------------------------------
CView* CControlCreateFunction (const UIAttributes& attributes, IViewFactory* factory, UIDescription* description)
{
	return 0;
}

//-----------------------------------------------------------------------------
bool CControlApplyAttributesFunction (CView* view, const UIAttributes& attributes, IViewFactory* factory, UIDescription* description)
{
	CControl* control = dynamic_cast<CControl*> (view);
	if (!control)
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
		long tag = description->getTagForName (controlTagAttr->c_str ());
		if (tag != -1)
		{
			control->setTag (tag);
			control->setListener (description->getControlListener (controlTagAttr->c_str ()));
		}
	}
	
	return true;
}

//-----------------------------------------------------------------------------
REGISTER_VIEW_CREATOR(CControl, "CControl", "CView", CControlCreateFunction, CControlApplyAttributesFunction)

//-----------------------------------------------------------------------------
// COnOffButton
//-----------------------------------------------------------------------------
CView* COnOffButtonCreateFunction (const UIAttributes& attributes, IViewFactory* factory, UIDescription* description)
{
	return new COnOffButton (CRect (0, 0, 0, 0), 0, -1, 0);
}

//-----------------------------------------------------------------------------
bool COnOffButtonApplyAttributesFunction (CView* view, const UIAttributes& attributes, IViewFactory* factory, UIDescription* description)
{
	return true;
}

//-----------------------------------------------------------------------------
REGISTER_VIEW_CREATOR(COnOffButton, "COnOffButton", "CControl", COnOffButtonCreateFunction, COnOffButtonApplyAttributesFunction)

//-----------------------------------------------------------------------------
// CParamDisplay
//-----------------------------------------------------------------------------
CView* CParamDisplayCreateFunction (const UIAttributes& attributes, IViewFactory* factory, UIDescription* description)
{
	return new CParamDisplay (CRect (0, 0, 0, 0));
}

//-----------------------------------------------------------------------------
bool CParamDisplayApplyAttributesFunction (CView* view, const UIAttributes& attributes, IViewFactory* factory, UIDescription* description)
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

	CColor color;
	if (fontAttr)
	{
		CFontRef font = description->getFont (fontAttr->c_str ());
		if (font)
			display->setFont (font);
	}
	if (fontColorAttr)
	{
		if (description->getColor (fontColorAttr->c_str (), color))
			display->setFontColor (color);
	}
	if (backColorAttr)
	{
		if (description->getColor (backColorAttr->c_str (), color))
			display->setBackColor (color);
	}
	if (frameColorAttr)
	{
		if (description->getColor (frameColorAttr->c_str (), color))
			display->setFrameColor (color);
	}
	if (shadowColorAttr)
	{
		if (description->getColor (shadowColorAttr->c_str (), color))
			display->setShadowColor (color);
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
	long style = 0;
	if (style3DInAttr && *style3DInAttr == "true")
		style |= k3DIn;
	if (style3DOutAttr && *style3DOutAttr == "true")
		style |= k3DOut;
	if (styleNoFrameAttr && *styleNoFrameAttr == "true")
		style |= kNoFrame;
	if (styleNoTextAttr && *styleNoTextAttr == "true")
		style |= kNoTextStyle;
	if (styleNoDrawAttr && *styleNoDrawAttr == "true")
		style |= kNoDrawStyle;
	if (styleShadowTextAttr && *styleShadowTextAttr == "true")
		style |= kShadowText;
	display->setStyle (style);

	return true;
}

//-----------------------------------------------------------------------------
REGISTER_VIEW_CREATOR(CParamDisplay, "CParamDisplay", "CControl", CParamDisplayCreateFunction, CParamDisplayApplyAttributesFunction)

//-----------------------------------------------------------------------------
// CTextLabel
//-----------------------------------------------------------------------------
CView* CTextLabelCreateFunction (const UIAttributes& attributes, IViewFactory* factory, UIDescription* description)
{
	return new CTextLabel (CRect (0, 0, 0, 0));
}

//-----------------------------------------------------------------------------
bool CTextLabelApplyAttributesFunction (CView* view, const UIAttributes& attributes, IViewFactory* factory, UIDescription* description)
{
	CTextLabel* label = dynamic_cast<CTextLabel*> (view);
	if (!label)
		return false;

	const std::string* titleAttr = attributes.getAttributeValue ("title");
	if (titleAttr)
		label->setText (titleAttr->c_str ());

	return true;
}

//-----------------------------------------------------------------------------
REGISTER_VIEW_CREATOR(CTextLabel, "CTextLabel", "CParamDisplay", CTextLabelCreateFunction, CTextLabelApplyAttributesFunction)

//-----------------------------------------------------------------------------
// CTextEdit
//-----------------------------------------------------------------------------
CView* CTextEditCreateFunction (const UIAttributes& attributes, IViewFactory* factory, UIDescription* description)
{
	return new CTextEdit (CRect (0, 0, 0, 0), 0, -1);
}

//-----------------------------------------------------------------------------
bool CTextEditApplyAttributesFunction (CView* view, const UIAttributes& attributes, IViewFactory* factory, UIDescription* description)
{
	CTextEdit* label = dynamic_cast<CTextEdit*> (view);
	if (!label)
		return false;

	const std::string* titleAttr = attributes.getAttributeValue ("title");
	if (titleAttr)
		label->setText (titleAttr->c_str ());

	return true;
}

//-----------------------------------------------------------------------------
REGISTER_VIEW_CREATOR(CTextEdit, "CTextEdit", "CParamDisplay", CTextEditCreateFunction, CTextEditApplyAttributesFunction)

//-----------------------------------------------------------------------------
// CKnob
//-----------------------------------------------------------------------------
CView* CKnobCreateFunction (const UIAttributes& attributes, IViewFactory* factory, UIDescription* description)
{
	return new CKnob (CRect (0, 0, 0, 0), 0, -1, 0, 0);
}

//-----------------------------------------------------------------------------
bool CKnobApplyAttributesFunction (CView* view, const UIAttributes& attributes, IViewFactory* factory, UIDescription* description)
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
			knob->setColorShadowHandle (color);
	}
	if (handleColorAttr)
	{
		if (description->getColor (handleColorAttr->c_str (), color))
			knob->setColorHandle (color);
	}
	if (handleBitmapAttr)
	{
		CBitmap* bitmap = description->getBitmap (handleBitmapAttr->c_str ());
		if (bitmap)
			knob->setHandleBitmap (bitmap);
	}

	return true;
}

//-----------------------------------------------------------------------------
REGISTER_VIEW_CREATOR(CKnob, "CKnob", "CControl", CKnobCreateFunction, CKnobApplyAttributesFunction)

//-----------------------------------------------------------------------------
bool IMultiBitmapApplyAttributesFunction (CView* view, const UIAttributes& attributes, IViewFactory* factory, UIDescription* description)
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
	return true;
}

//-----------------------------------------------------------------------------
// CAnimKnob
//-----------------------------------------------------------------------------
CView* CAnimKnobCreateFunction (const UIAttributes& attributes, IViewFactory* factory, UIDescription* description)
{
	return new CAnimKnob (CRect (0, 0, 0, 0), 0, -1, 0);
}

//-----------------------------------------------------------------------------
REGISTER_VIEW_CREATOR(CAnimKnob, "CAnimKnob", "CKnob", CAnimKnobCreateFunction, IMultiBitmapApplyAttributesFunction)

//-----------------------------------------------------------------------------
// CVerticalSwitch
//-----------------------------------------------------------------------------
CView* CVerticalSwitchCreateFunction (const UIAttributes& attributes, IViewFactory* factory, UIDescription* description)
{
	return new CVerticalSwitch (CRect (0, 0, 0, 0), 0, -1, 0);
}

//-----------------------------------------------------------------------------
REGISTER_VIEW_CREATOR(CVerticalSwitch, "CVerticalSwitch", "CControl", CVerticalSwitchCreateFunction, IMultiBitmapApplyAttributesFunction)

//-----------------------------------------------------------------------------
// CHorizontalSwitch
//-----------------------------------------------------------------------------
CView* CHorizontalSwitchCreateFunction (const UIAttributes& attributes, IViewFactory* factory, UIDescription* description)
{
	return new CHorizontalSwitch (CRect (0, 0, 0, 0), 0, -1, 0);
}

//-----------------------------------------------------------------------------
REGISTER_VIEW_CREATOR(CHorizontalSwitch, "CHorizontalSwitch", "CControl", CHorizontalSwitchCreateFunction, IMultiBitmapApplyAttributesFunction)

//-----------------------------------------------------------------------------
// CRockerSwitch
//-----------------------------------------------------------------------------
CView* CRockerSwitchCreateFunction (const UIAttributes& attributes, IViewFactory* factory, UIDescription* description)
{
	return new CRockerSwitch (CRect (0, 0, 0, 0), 0, -1, 0);
}

//-----------------------------------------------------------------------------
REGISTER_VIEW_CREATOR(CRockerSwitch, "CRockerSwitch", "CControl", CRockerSwitchCreateFunction, IMultiBitmapApplyAttributesFunction)

//-----------------------------------------------------------------------------
// CMovieBitmap
//-----------------------------------------------------------------------------
CView* CMovieBitmapCreateFunction (const UIAttributes& attributes, IViewFactory* factory, UIDescription* description)
{
	return new CMovieBitmap (CRect (0, 0, 0, 0), 0, -1, 0);
}

//-----------------------------------------------------------------------------
REGISTER_VIEW_CREATOR(CMovieBitmap, "CMovieBitmap", "CControl", CMovieBitmapCreateFunction, IMultiBitmapApplyAttributesFunction)

//-----------------------------------------------------------------------------
// CMovieButton
//-----------------------------------------------------------------------------
CView* CMovieButtonCreateFunction (const UIAttributes& attributes, IViewFactory* factory, UIDescription* description)
{
	return new CMovieButton (CRect (0, 0, 0, 0), 0, -1, 0);
}

//-----------------------------------------------------------------------------
REGISTER_VIEW_CREATOR(CMovieButton, "CMovieButton", "CControl", CMovieButtonCreateFunction, IMultiBitmapApplyAttributesFunction)

//-----------------------------------------------------------------------------
// CKickButton
//-----------------------------------------------------------------------------
CView* CKickButtonCreateFunction (const UIAttributes& attributes, IViewFactory* factory, UIDescription* description)
{
	return new CKickButton (CRect (0, 0, 0, 0), 0, -1, 0);
}

//-----------------------------------------------------------------------------
REGISTER_VIEW_CREATOR(CKickButton, "CKickButton", "CControl", CKickButtonCreateFunction, IMultiBitmapApplyAttributesFunction)

//-----------------------------------------------------------------------------
// CSlider
//-----------------------------------------------------------------------------
CView* CSliderCreateFunction (const UIAttributes& attributes, IViewFactory* factory, UIDescription* description)
{
	return new CSlider (CRect (0, 0, 0, 0), 0, -1, 0, 0, 0, 0);
}

//-----------------------------------------------------------------------------
bool CSliderApplyAttributesFunction (CView* view, const UIAttributes& attributes, IViewFactory* factory, UIDescription* description)
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
	long style = kHorizontal;
	if (orientationAttr)
	{
		if (*orientationAttr == "vertical")
			style = kVertical;
	}
	if (reverseOrientationAttr && *reverseOrientationAttr == "true")
	{
		if (style & kVertical)
			style |= kTop;
		else if (style & kHorizontal)
			style |= kRight;
	}
	else
	{
		if (style & kVertical)
			style |= kBottom;
		else if (style & kHorizontal)
			style |= kLeft;
	}
	slider->setStyle (style);
	return true;
}

//-----------------------------------------------------------------------------
REGISTER_VIEW_CREATOR(CSlider, "CSlider", "CControl", CSliderCreateFunction, CSliderApplyAttributesFunction)

END_NAMESPACE_VSTGUI

/**
@endcond ignore
*/