#include "uicolorscontroller.h"

#if VSTGUI_LIVE_EDITING

#include "uieditcontroller.h"
#include "uisearchtextfield.h"
#include "uibasedatasource.h"
#include "../../lib/controls/cslider.h"
#include "../../lib/coffscreencontext.h"
#include <sstream>

namespace VSTGUI {

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
class UIColor : public CColor, public CBaseObject, public IDependency
{
public:
	UIColor () : CColor (kTransparentCColor) {}

	UIColor& operator= (const CColor& c)
	{
		red = c.red; green = c.green, blue = c.blue; alpha = c.alpha; 
		r = red; g = green; b = blue;
		updateHSL ();
		return *this;
	}

	void updateHSL ()
	{
		toHSL (hue, saturation, lightness);
	}

	double hue, saturation, lightness;
	double r, g, b;

	static IdStringPtr kMsgChanged;
	static IdStringPtr kMsgEditChange;
	static IdStringPtr kMsgBeginEditing;
	static IdStringPtr kMsgEndEditing;
};

//----------------------------------------------------------------------------------------------------
IdStringPtr UIColor::kMsgChanged = "UIColor::kMsgChanged";
IdStringPtr UIColor::kMsgEditChange = "UIColor::kMsgEditChange";
IdStringPtr UIColor::kMsgBeginEditing = "UIColor::kMsgBeginEditing";
IdStringPtr UIColor::kMsgEndEditing = "UIColor::kMsgEndEditing";

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
class UIColorSlider : public CSlider
{
public:
	enum {
		kHue,
		kSaturation,
		kLightness,
		kRed,
		kGreen,
		kBlue,
		kAlpha,
	};
	UIColorSlider (UIColor* color, int32_t style);
	~UIColorSlider ();

protected:
	void draw (CDrawContext* context)
	{
		if (getHandle () == 0)
			updateBackground ();
		CSlider::draw (context);
	}
	void setViewSize (const CRect& rect, bool invalid = true)
	{
		bool different = rect != getViewSize ();
		CSlider::setViewSize (rect, invalid);
		if (different)
			updateBackground ();
	}
	CMessageResult notify (CBaseObject* sender, IdStringPtr message);
	void updateBackground ();

	SharedPointer<UIColor> color;
	int32_t style;
};

//----------------------------------------------------------------------------------------------------
UIColorSlider::UIColorSlider (UIColor* color, int32_t style)
: CSlider (CRect (0, 0, 0, 0), 0, 0, 0, 0, 0, 0)
, color (color)
, style (style)
{
	color->addDependency (this);
}

//----------------------------------------------------------------------------------------------------
UIColorSlider::~UIColorSlider ()
{
	color->removeDependency (this);
}

//----------------------------------------------------------------------------------------------------
CMessageResult UIColorSlider::notify (CBaseObject* sender, IdStringPtr message)
{
	if (message == UIColor::kMsgChanged || message == UIColor::kMsgEditChange)
	{
		updateBackground ();
		return kMessageNotified;
	}
	return CSlider::notify (sender, message);
}

//----------------------------------------------------------------------------------------------------
void UIColorSlider::updateBackground ()
{
	setBackground (0);
	COffscreenContext* context = COffscreenContext::create (getFrame (), getWidth (), getHeight ());
	if (context)
	{
		CCoord width = getWidth ();
		context->beginDraw ();
		context->setDrawMode (kAliasing);
		CCoord widthPerColor = width / 256.;
		CRect r;
		r.setHeight (getHeight ());
		for (int32_t i = 0; i < 256; i++)
		{
			CCoord x = floor (widthPerColor * i + 0.5);
			if (x > r.right)
			{
				r.left = r.right;
				r.setWidth (widthPerColor < 2. ? 2. : ceil (widthPerColor));
				CColor c = *color;
				switch (style)
				{
					case kRed:
					{
						c.red = (int8_t)i;
						break;
					}
					case kGreen:
					{
						c.green = (int8_t)i;
						break;
					}
					case kBlue:
					{
						c.blue = (int8_t)i;
						break;
					}
					case kAlpha:
					{
						c.alpha = (int8_t)i;
						break;
					}
					case kHue:
					{
						double hue = ((double)i / 256.) * 360.;
						c.fromHSL (hue, color->saturation, color->lightness);
						break;
					}
					case kSaturation:
					{
						double sat = ((double)i / 256.);
						c.fromHSL (color->hue, sat, color->lightness);
						break;
					}
					case kLightness:
					{
						double light = ((double)i / 256.);
						c.fromHSL (color->hue, color->saturation, light);
						break;
					}
				}
				context->setFillColor (c);
				context->drawRect (r, kDrawFilled);
			}
		}
		context->endDraw ();
		setBackground (context->getBitmap());
		context->forget ();
	}
	if (getHandle () == 0)
	{
		context = COffscreenContext::create (getFrame (), 7, getHeight ());
		if (context)
		{
			context->beginDraw ();
			context->setFrameColor (kBlackCColor);
			context->setLineWidth (1);
			context->setDrawMode (kAliasing);
			CRect r (0, 0, 7, getHeight ());
			context->drawRect (r, kDrawStroked);
			r.inset (1, 1);
			context->setFrameColor (kWhiteCColor);
			context->drawRect (r, kDrawStroked);
			context->endDraw ();
			setHandle (context->getBitmap ());
			context->forget ();
		}
		
	}
}

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
class UIColorChooserController : public CBaseObject, public DelegationController
{
public:
	UIColorChooserController (IController* baseController, UIColor* color);
	~UIColorChooserController ();
	
protected:
	CMessageResult notify (CBaseObject* sender, IdStringPtr message);
	CView* createView (const UIAttributes& attributes, IUIDescription* description);
	CView* verifyView (CView* view, const UIAttributes& attributes, IUIDescription* description);
	CControlListener* getControlListener (UTF8StringPtr name);
	void valueChanged (CControl* pControl);
	void controlBeginEdit (CControl* pControl);
	void controlEndEdit (CControl* pControl);

	static bool valueToString (float value, char utf8String[256], void* userData);
	static bool stringToValue (UTF8StringPtr txt, float& result, void* userData);

	SharedPointer<UIColor> color;
	std::list<SharedPointer<CControl> > controls;

	enum {
		kHueTag = 0,
		kSaturationTag,
		kLightnessTag,
		kRedTag,
		kGreenTag,
		kBlueTag,
		kAlphaTag,
	};
};

//----------------------------------------------------------------------------------------------------
UIColorChooserController::UIColorChooserController (IController* baseController, UIColor* color)
: DelegationController (baseController)
, color (color)
{
	color->addDependency (this);
}

//----------------------------------------------------------------------------------------------------
UIColorChooserController::~UIColorChooserController ()
{
	color->removeDependency (this);
}

//----------------------------------------------------------------------------------------------------
CMessageResult UIColorChooserController::notify (CBaseObject* sender, IdStringPtr message)
{
	if (message == UIColor::kMsgChanged)
	{
		for (std::list<SharedPointer<CControl> >::const_iterator it = controls.begin (); it != controls.end (); it++)
		{
			float value = 0.f;
			switch ((*it)->getTag ())
			{
				case kHueTag:
				{
					value = (float)color->hue;
					break;
				}
				case kSaturationTag:
				{
					value = (float)color->saturation;
					break;
				}
				case kLightnessTag:
				{
					value = (float)color->lightness;
					break;
				}
				case kRedTag:
				{
					value = (float)color->red;
					break;
				}
				case kGreenTag:
				{
					value = (float)color->green;
					break;
				}
				case kBlueTag:
				{
					value = (float)color->blue;
					break;
				}
				case kAlphaTag:
				{
					value = (float)color->alpha;
					break;
				}
				default:
					continue;
			}
			(*it)->setValue (value);
			(*it)->invalid ();
		}
	}
	return kMessageUnknown;
}

//----------------------------------------------------------------------------------------------------
bool UIColorChooserController::valueToString (float value, char utf8String[256], void* userData)
{
	CParamDisplay* display = static_cast<CParamDisplay*>(userData);
	std::stringstream str;
	switch (display->getTag ())
	{
		case kSaturationTag:
		case kLightnessTag:
		{
			str << (uint32_t)(value * 100);
			str << " %";
			break;
		}
		case kHueTag:
		{
			str << (uint32_t)value;
			str << kDegreeSymbol;
			break;
		}
		default:
		{
			str << (uint32_t)value;
			break;
		}
	}
	strncpy (utf8String, str.str ().c_str (), 255);
	return true;
}

//----------------------------------------------------------------------------------------------------
bool UIColorChooserController::stringToValue (UTF8StringPtr txt, float& result, void* userData)
{
	char* endptr = 0;
	result = (float)strtod (txt, &endptr);
	if (endptr != txt)
	{
		CParamDisplay* display = static_cast<CParamDisplay*>(userData);
		switch (display->getTag ())
		{
			case kSaturationTag:
			case kLightnessTag:
			{
				result /= 100.f;
				break;
			}
			default:
				break;
		}
		return true;
	}
	return false;
}

//----------------------------------------------------------------------------------------------------
CView* UIColorChooserController::createView (const UIAttributes& attributes, IUIDescription* description)
{
	const std::string* name = attributes.getAttributeValue ("custom-view-name");
	if (name)
	{
		if (*name == "UIColorSlider")
		{
			const std::string* controlTagStr = attributes.getAttributeValue ("control-tag");
			int32_t tag = controlTagStr ? description->getTagForName (controlTagStr->c_str ()) : -1;
			if (tag != -1)
			{
				return new UIColorSlider (color, tag);
			}
		}
	}

	return DelegationController::createView (attributes, description);
}

//----------------------------------------------------------------------------------------------------
CView* UIColorChooserController::verifyView (CView* view, const UIAttributes& attributes, IUIDescription* description)
{
	CControl* control = dynamic_cast<CControl*>(view);
	if (control && control->getTag () >= 0)
	{
		controls.push_back (control);
		CTextEdit* textEdit = dynamic_cast<CTextEdit*> (control);
		if (textEdit)
		{
			textEdit->setValueToStringProc (valueToString, textEdit);
			textEdit->setStringToValueProc (stringToValue, textEdit);
		}
	}
	return view;
}

//----------------------------------------------------------------------------------------------------
CControlListener* UIColorChooserController::getControlListener (UTF8StringPtr name)
{
	return this;
}

//----------------------------------------------------------------------------------------------------
void UIColorChooserController::controlBeginEdit (CControl* pControl)
{
	if (pControl->getTag () >= kHueTag && pControl->getTag () <= kAlphaTag)
	{
		color->changed (UIColor::kMsgBeginEditing);
	}
}

//----------------------------------------------------------------------------------------------------
void UIColorChooserController::controlEndEdit (CControl* pControl)
{
	if (pControl->getTag () >= kHueTag && pControl->getTag () <= kAlphaTag)
	{
		color->changed (UIColor::kMsgEndEditing);
	}
}

//----------------------------------------------------------------------------------------------------
void UIColorChooserController::valueChanged (CControl* pControl)
{
	switch (pControl->getTag ())
	{
		case kHueTag:
		{
			color->hue = pControl->getValue ();
			color->fromHSL (color->hue, color->saturation, color->lightness);
			break;
		}
		case kSaturationTag:
		{
			color->saturation = pControl->getValue ();
			color->fromHSL (color->hue, color->saturation, color->lightness);
			break;
		}
		case kLightnessTag:
		{
			color->lightness = pControl->getValue ();
			color->fromHSL (color->hue, color->saturation, color->lightness);
			break;
		}
		case kRedTag:
		{
			color->r = pControl->getValue ();
			color->updateHSL ();
			color->red = (uint8_t)color->r;
			break;
		}
		case kGreenTag:
		{
			color->g = (uint8_t)pControl->getValue ();
			color->updateHSL ();
			color->green = (uint8_t)color->g;
			break;
		}
		case kBlueTag:
		{
			color->b = (uint8_t)pControl->getValue ();
			color->updateHSL ();
			color->blue = (uint8_t)color->b;
			break;
		}
		case kAlphaTag:
		{
			color->alpha = (uint8_t)pControl->getValue ();
			break;
		}
		default:
			return;
	}
	notify (color, UIColor::kMsgChanged);
	color->changed (UIColor::kMsgEditChange);
}

//----------------------------------------------------------------------------------------------------
class UIColorsDataSource : public UIBaseDataSource
{
public:
	UIColorsDataSource (UIDescription* description, IActionPerformer* actionPerformer, UIColor* color);
	~UIColorsDataSource ();
	
protected:
	CMessageResult notify (CBaseObject* sender, IdStringPtr message);
	virtual void update ();
	virtual void getNames (std::list<const std::string*>& names);
	virtual bool addItem (UTF8StringPtr name);
	virtual bool removeItem (UTF8StringPtr name);
	virtual bool performNameChange (UTF8StringPtr oldName, UTF8StringPtr newName);
	virtual UTF8StringPtr getDefaultsName () { return "UIColorsDataSource"; }

	void dbDrawCell (CDrawContext* context, const CRect& size, int32_t row, int32_t column, int32_t flags, CDataBrowser* browser);
	void dbSelectionChanged (CDataBrowser* browser);

	SharedPointer<UIColor> color;
	bool editing;
};

//----------------------------------------------------------------------------------------------------
UIColorsDataSource::UIColorsDataSource (UIDescription* description, IActionPerformer* actionPerformer, UIColor* color)
: UIBaseDataSource (description, actionPerformer, UIDescription::kMessageColorChanged)
, color (color)
, editing (false)
{
	color->addDependency (this);
}

//----------------------------------------------------------------------------------------------------
UIColorsDataSource::~UIColorsDataSource ()
{
	color->removeDependency (this);
}

//----------------------------------------------------------------------------------------------------
CMessageResult UIColorsDataSource::notify (CBaseObject* sender, IdStringPtr message)
{
	CMessageResult result = UIBaseDataSource::notify (sender, message);
	if (result != kMessageNotified)
	{
		if (message == UIColor::kMsgBeginEditing)
		{
			int32_t selectedRow = dataBrowser->getSelectedRow ();
			if (selectedRow != CDataBrowser::kNoSelection)
			{
				actionPerformer->beginLiveColorChange (names.at (selectedRow).c_str ());
				editing = true;
			}
		}
		else if (message == UIColor::kMsgEndEditing)
		{
			int32_t selectedRow = dataBrowser->getSelectedRow ();
			if (selectedRow != CDataBrowser::kNoSelection)
			{
				actionPerformer->endLiveColorChange (names.at (selectedRow).c_str ());
				editing = false;
			}
		}
		else if (message == UIColor::kMsgEditChange)
		{
			if (editing)
			{
				int32_t selectedRow = dataBrowser->getSelectedRow ();
				if (selectedRow != CDataBrowser::kNoSelection)
				{
					actionPerformer->performLiveColorChange (names.at (selectedRow).c_str (), *color);
					dataBrowser->setSelectedRow (selectedRow);
				}
			}
		}
	}
	return result;
}

//----------------------------------------------------------------------------------------------------
void UIColorsDataSource::getNames (std::list<const std::string*>& names)
{
	description->collectColorNames (names);
}

//----------------------------------------------------------------------------------------------------
bool UIColorsDataSource::addItem (UTF8StringPtr name)
{
	actionPerformer->performColorChange (name, kWhiteCColor);
	return true;
}

//----------------------------------------------------------------------------------------------------
bool UIColorsDataSource::removeItem (UTF8StringPtr name)
{
	actionPerformer->performColorChange (name, kWhiteCColor, true);
	return true;
}

//----------------------------------------------------------------------------------------------------
void UIColorsDataSource::update ()
{
	UIBaseDataSource::update ();
	if (dataBrowser)
	{
		dbSelectionChanged (dataBrowser);
		dataBrowser->invalid ();
	}
}

//----------------------------------------------------------------------------------------------------
void UIColorsDataSource::dbSelectionChanged (CDataBrowser* browser)
{
	int32_t selectedRow = dataBrowser->getSelectedRow ();
	if (selectedRow != CDataBrowser::kNoSelection)
	{
		CColor c;
		if (description->getColor (names.at (selectedRow).c_str (), c))
		{
			if (c != *color)
			{
				*color = c;
				color->changed (UIColor::kMsgChanged);
			}
		}
	}
}

//----------------------------------------------------------------------------------------------------
void UIColorsDataSource::dbDrawCell (CDrawContext* context, const CRect& size, int32_t row, int32_t column, int32_t flags, CDataBrowser* browser)
{
	GenericStringListDataBrowserSource::dbDrawCell (context, size, row, column, flags, browser);
	CColor color;
	if (description->getColor(names.at (row).c_str (), color))
	{
		context->setFillColor (color);
		context->setFrameColor (kBlackCColor);
		context->setLineWidth (1);
		context->setLineStyle (kLineSolid);
		context->setDrawMode (kAliasing);
		CRect r (size);
		r.left = r.right - r.getHeight ();
		r.inset (2, 2);
		context->drawRect (r, kDrawFilledAndStroked);
	}
}

//----------------------------------------------------------------------------------------------------
bool UIColorsDataSource::performNameChange (UTF8StringPtr oldName, UTF8StringPtr newName)
{
	actionPerformer->performColorNameChange (oldName, newName);
	return true;
}

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
UIColorsController::UIColorsController (IController* baseController, UIDescription* description, IActionPerformer* actionPerformer)
: DelegationController (baseController)
, editDescription (description)
, actionPerformer (actionPerformer)
, dataSource (0)
, color (new UIColor)
{
	dataSource = new UIColorsDataSource (editDescription, actionPerformer, color);
	UIEditController::setupDataSource (dataSource);
}

//----------------------------------------------------------------------------------------------------
UIColorsController::~UIColorsController ()
{
	dataSource->forget ();
}

//----------------------------------------------------------------------------------------------------
CView* UIColorsController::createView (const UIAttributes& attributes, IUIDescription* description)
{
	const std::string* name = attributes.getAttributeValue ("custom-view-name");
	if (name)
	{
		if (*name == "ColorsBrowser")
		{
			CDataBrowser* dataBrowser = new CDataBrowser (CRect (0, 0, 0, 0), 0, dataSource, CDataBrowser::kDrawRowLines|CScrollView::kHorizontalScrollbar | CScrollView::kVerticalScrollbar);
			return dataBrowser;
		}
		#if 0
		else if (*name == "ColorChooser")
		{
			CColorChooserUISettings ui;
			ui.font = description->getFont("colorchooser.font");
			ui.fontColor = kBlackCColor;
			ui.margin = CPoint (2, 2);
			ui.checkerBoardBack = true;
			ui.checkerBoardColor1 = kWhiteCColor;
			ui.checkerBoardColor2 = kGreyCColor;
			CColorChooser* colorChooser = new CColorChooser (dataSource, kWhiteCColor, ui);
			dataSource->setColorChooser (colorChooser);
			return colorChooser;
		}
		#endif
	}
	return DelegationController::createView (attributes, description);
}

//----------------------------------------------------------------------------------------------------
CView* UIColorsController::verifyView (CView* view, const UIAttributes& attributes, IUIDescription* description)
{
	UISearchTextField* searchField = dynamic_cast<UISearchTextField*>(view);
	if (searchField && searchField->getTag () == kSearchTag)
	{
		dataSource->setSearchFieldControl (searchField);
		return searchField;
	}
	return controller->verifyView (view, attributes, description);
}

//----------------------------------------------------------------------------------------------------
CControlListener* UIColorsController::getControlListener (UTF8StringPtr name)
{
	return this;
}

//----------------------------------------------------------------------------------------------------
void UIColorsController::valueChanged (CControl* pControl)
{
	switch (pControl->getTag ())
	{
		case kAddTag:
		{
			if (pControl->getValue () == pControl->getMax ())
			{
				dataSource->add ();
			}
			break;
		}
		case kRemoveTag:
		{
			if (pControl->getValue () == pControl->getMax ())
			{
				dataSource->remove ();
			}
			break;
		}
	}
}

//----------------------------------------------------------------------------------------------------
IController* UIColorsController::createSubController (IdStringPtr name, IUIDescription* description)
{
	if (strcmp (name, "ColorChooserController") == 0)
		return new UIColorChooserController (this, color);
	return controller->createSubController (name, description);
}

} // namespace

#endif // VSTGUI_LIVE_EDITING
