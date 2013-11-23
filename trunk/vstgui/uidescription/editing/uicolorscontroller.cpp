//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework not only for VST plugins
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

#include "uicolorscontroller.h"

#if VSTGUI_LIVE_EDITING

#include "uieditcontroller.h"
#include "uisearchtextfield.h"
#include "uibasedatasource.h"
#include "../../lib/controls/cslider.h"
#include "../../lib/coffscreencontext.h"
#include "../../lib/idatapackage.h"
#include <sstream>

namespace VSTGUI {

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
class UIColor : public CColor, public CBaseObject, public IDependency
{
public:
	UIColor () : CColor (kTransparentCColor), hue (0), saturation (0), lightness (0) {}

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
		kAlpha
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
	COffscreenContext* context = COffscreenContext::create (getFrame (), floor (getWidth () + 0.5), floor (getHeight () + 0.5));
	if (context)
	{
		CCoord width = getWidth ();
		context->beginDraw ();
		context->setDrawMode (kAliasing);
		CCoord widthPerColor = width / 256.;
		CRect r;
		r.setHeight (floor (getHeight () + 0.5));
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
		kAlphaTag
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
			color->red = (uint8_t)color->r;
			color->updateHSL ();
			break;
		}
		case kGreenTag:
		{
			color->g = (uint8_t)pControl->getValue ();
			color->green = (uint8_t)color->g;
			color->updateHSL ();
			break;
		}
		case kBlueTag:
		{
			color->b = (uint8_t)pControl->getValue ();
			color->blue = (uint8_t)color->b;
			color->updateHSL ();
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
	virtual void update () VSTGUI_OVERRIDE_VMETHOD;
	virtual void getNames (std::list<const std::string*>& names) VSTGUI_OVERRIDE_VMETHOD;
	virtual bool addItem (UTF8StringPtr name) VSTGUI_OVERRIDE_VMETHOD;
	virtual bool removeItem (UTF8StringPtr name) VSTGUI_OVERRIDE_VMETHOD;
	virtual bool performNameChange (UTF8StringPtr oldName, UTF8StringPtr newName) VSTGUI_OVERRIDE_VMETHOD;
	virtual UTF8StringPtr getDefaultsName () VSTGUI_OVERRIDE_VMETHOD { return "UIColorsDataSource"; }

	void dbDrawCell (CDrawContext* context, const CRect& size, int32_t row, int32_t column, int32_t flags, CDataBrowser* browser) VSTGUI_OVERRIDE_VMETHOD;
	void dbCellSetupTextEdit (int32_t row, int32_t column, CTextEdit* control, CDataBrowser* browser) VSTGUI_OVERRIDE_VMETHOD;
	void dbSelectionChanged (CDataBrowser* browser) VSTGUI_OVERRIDE_VMETHOD;

	void dbOnDragEnterBrowser (IDataPackage* drag, CDataBrowser* browser) VSTGUI_OVERRIDE_VMETHOD;
	void dbOnDragExitBrowser (IDataPackage* drag, CDataBrowser* browser) VSTGUI_OVERRIDE_VMETHOD;
	void dbOnDragEnterCell (int32_t row, int32_t column, const CPoint& where, IDataPackage* drag, CDataBrowser* browser) VSTGUI_OVERRIDE_VMETHOD;
	void dbOnDragMoveInCell (int32_t row, int32_t column, const CPoint& where, IDataPackage* drag, CDataBrowser* browser) VSTGUI_OVERRIDE_VMETHOD;
	void dbOnDragExitCell (int32_t row, int32_t column, IDataPackage* drag, CDataBrowser* browser) VSTGUI_OVERRIDE_VMETHOD;
	bool dbOnDropInCell (int32_t row, int32_t column, const CPoint& where, IDataPackage* drag, CDataBrowser* browser) VSTGUI_OVERRIDE_VMETHOD;

	SharedPointer<UIColor> color;
	bool editing;
	std::string colorString;
	int32_t dragRow;
};

//----------------------------------------------------------------------------------------------------
UIColorsDataSource::UIColorsDataSource (UIDescription* description, IActionPerformer* actionPerformer, UIColor* color)
: UIBaseDataSource (description, actionPerformer, UIDescription::kMessageColorChanged)
, color (color)
, editing (false)
, dragRow (-1)
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
	if (description->getColor (names.at (row).c_str (), color))
	{
		context->setFillColor (color);
		context->setFrameColor (dragRow == row ? kRedCColor : kBlackCColor);
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
void UIColorsDataSource::dbCellSetupTextEdit (int32_t row, int32_t column, CTextEdit* control, CDataBrowser* browser)
{
	UIBaseDataSource::dbCellSetupTextEdit(row, column, control, browser);
	CRect r (control->getViewSize ());
	r.right -= r.getHeight ();
	control->setViewSize (r);
}


//----------------------------------------------------------------------------------------------------
bool UIColorsDataSource::performNameChange (UTF8StringPtr oldName, UTF8StringPtr newName)
{
	actionPerformer->performColorNameChange (oldName, newName);
	return true;
}

//----------------------------------------------------------------------------------------------------
void UIColorsDataSource::dbOnDragEnterBrowser (IDataPackage* drag, CDataBrowser* browser)
{
	IDataPackage::Type type;
	const void* item;
	if (drag->getData (0, item, type) > 0 && type == IDataPackage::kText)
	{
		std::string tmp (static_cast<const char*>(item));
		if (tmp.length () == 9 && tmp[0] == '#')
		{
			colorString = tmp;
			browser->getFrame ()->setCursor (kCursorCopy);
		}
	}
}

//----------------------------------------------------------------------------------------------------
void UIColorsDataSource::dbOnDragExitBrowser (IDataPackage* drag, CDataBrowser* browser)
{
	if (colorString.length () > 0)
	{
		colorString = "";
		browser->getFrame ()->setCursor (kCursorNotAllowed);
	}
}

//----------------------------------------------------------------------------------------------------
void UIColorsDataSource::dbOnDragEnterCell (int32_t row, int32_t column, const CPoint& where, IDataPackage* drag, CDataBrowser* browser)
{
	if (colorString.length () > 0 && row >= 0)
	{
		browser->getFrame()->setCursor (kCursorDefault);
		dragRow = row;
		browser->invalidateRow (dragRow);
	}
}

//----------------------------------------------------------------------------------------------------
void UIColorsDataSource::dbOnDragMoveInCell (int32_t row, int32_t column, const CPoint& where, IDataPackage* drag, CDataBrowser* browser)
{
	if (dragRow == dbGetNumRows (browser))
	{
		
	}
}

//----------------------------------------------------------------------------------------------------
void UIColorsDataSource::dbOnDragExitCell (int32_t row, int32_t column, IDataPackage* drag, CDataBrowser* browser)
{
	if (colorString.length () > 0)
	{
		browser->getFrame()->setCursor (kCursorCopy);
		if (dragRow >= 0)
			browser->invalidateRow (dragRow);
		dragRow = -1;
	}
}

//----------------------------------------------------------------------------------------------------
bool UIColorsDataSource::dbOnDropInCell (int32_t row, int32_t column, const CPoint& where, IDataPackage* drag, CDataBrowser* browser)
{
	if (colorString.length () > 0)
	{
		CColor dragColor;
		std::string rv (colorString.substr (1, 2));
		std::string gv (colorString.substr (3, 2));
		std::string bv (colorString.substr (5, 2));
		std::string av (colorString.substr (7, 2));
		dragColor.red = (uint8_t)strtol (rv.c_str (), 0, 16);
		dragColor.green = (uint8_t)strtol (gv.c_str (), 0, 16);
		dragColor.blue = (uint8_t)strtol (bv.c_str (), 0, 16);
		dragColor.alpha = (uint8_t)strtol (av.c_str (), 0, 16);
		
		if (row >= 0)
		{
			actionPerformer->performColorChange (names[row].c_str (), dragColor);
		}
		else
		{
			std::string newName (filterString.empty () ? "New" : filterString);
			if (createUniqueName (newName))
			{
				actionPerformer->performColorChange (newName.c_str (), dragColor);
			}
		}
		return true;
	}
	return false;
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
			CDataBrowser* dataBrowser = new CDataBrowser (CRect (0, 0, 0, 0), dataSource, CDataBrowser::kDrawRowLines|CScrollView::kHorizontalScrollbar|CScrollView::kVerticalScrollbar);
			return dataBrowser;
		}
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
