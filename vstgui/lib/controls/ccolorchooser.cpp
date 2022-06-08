// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "ccolorchooser.h"
#include "cslider.h"
#include "ctextlabel.h"
#include "ccontrol.h"
#include "../cdrawcontext.h"
#include "../cframe.h"
#include "../idatapackage.h"
#include "../dragging.h"
#include <string>

namespace VSTGUI {

/// @cond ignore
namespace CColorChooserInternal {

//-----------------------------------------------------------------------------
class Slider : public CSlider
{
public:
	Slider (const CRect& size, IControlListener* listener = nullptr, int32_t tag = -1)
	: CSlider (size, listener, tag, 0, 0, nullptr, nullptr)
	{
		if (size.getWidth () > size.getHeight ())
			setHandleSizePrivate (size.getHeight (), size.getHeight ());
		else
			setHandleSizePrivate (size.getWidth (), size.getWidth ());
		const CRect& r (size);
		setViewSize (r, false);
		setWheelInc (10.f/255.f);
	}

	void draw (CDrawContext* context) override
	{
		CColor handleFillColor (kWhiteCColor);
		CColor handleFrameColor (kBlackCColor);
		CColor backgroundFillColor (kGreyCColor);
		CColor backgroundFrameColor (kBlackCColor);
		CColor bandColor (kTransparentCColor);

		CCoord backgroundFrameWidth = 1;
		CCoord handleFrameWidth = 1;

		auto controlSize = getControlSizePrivate ();
		auto sliderSize = getHandleSizePrivate ();

		CRect backgroundRect;
		backgroundRect.setSize (controlSize);
		backgroundRect.offset (getViewSize ().left, getViewSize ().top);
		context->setDrawMode (kAntiAliasing);
		context->setFillColor (backgroundFillColor);
		context->setFrameColor (backgroundFrameColor);
		context->setLineWidth (backgroundFrameWidth);
		context->setLineStyle (kLineSolid);
		context->drawRect (backgroundRect, kDrawFilledAndStroked);
		
		if (getStyle () & kHorizontal)
		{
			backgroundRect.left += getOffsetHandle ().x + sliderSize.x / 2;
			backgroundRect.right -= getOffsetHandle ().x + sliderSize.x / 2;
			backgroundRect.top += controlSize.y / 2 - 2;
			backgroundRect.bottom -= controlSize.y / 2 - 2;
		}
		else
		{
			backgroundRect.left += controlSize.x / 2 - 2;
			backgroundRect.right -= controlSize.x / 2 - 2;
			backgroundRect.top += getOffsetHandle ().y + sliderSize.y / 2;
			backgroundRect.bottom -= getOffsetHandle ().y + sliderSize.y / 2;
		}
		context->setFillColor (bandColor);
		context->drawRect (backgroundRect, kDrawFilled);

		// calc new coords of slider
		CRect rectNew = calculateHandleRect (getValueNormalized ());

		context->setFillColor (handleFillColor);
		context->setFrameColor (handleFrameColor);
		context->setLineWidth (handleFrameWidth);
		context->drawRect (rectNew, kDrawFilledAndStroked);

		setDirty (false);
	}

};

//-----------------------------------------------------------------------------
class ColorView : public CControl, public IDropTarget
{
public:
	ColorView (const CRect& r, const CColor& initialColor, IControlListener* listener = nullptr, int32_t tag = -1, bool checkerBoardBack = true, const CColor& checkerBoardColor1 = kWhiteCColor, const CColor& checkerBoardColor2 = kBlackCColor)
	: CControl (r, listener, tag)
	, color (initialColor)
	, checkerBoardColor1 (checkerBoardColor1)
	, checkerBoardColor2 (checkerBoardColor2)
	, checkerBoardBack (checkerBoardBack)
	{
	}
	
	void draw (CDrawContext* context) override
	{
		context->setDrawMode (kAliasing);
		if (checkerBoardBack && color.alpha != 255)
		{
			context->setFillColor (checkerBoardColor1);
			context->drawRect (getViewSize (), kDrawFilled);
			context->setFillColor (checkerBoardColor2);
			CRect r (getViewSize ().left, getViewSize ().top, getViewSize ().left + 5, getViewSize ().top + 5);
			for (int32_t x = 0; x < getViewSize ().getWidth (); x+=5)
			{
				r.left = getViewSize ().left + x;
				r.top = (x % 2) ? getViewSize ().top : getViewSize ().top + 5;
				r.right = r.left + 5;
				r.bottom = r.top + 5;
				for (int32_t y = 0; y < getViewSize ().getHeight (); y+=10)
				{
					context->drawRect (r, kDrawFilled);
					r.offset (0, 10);
				}
			}
		}
		context->setLineWidth (1);
		context->setFillColor (color);
		context->setFrameColor (kBlackCColor);
		context->drawRect (getViewSize (), kDrawFilledAndStroked);
				
		setDirty (false);
	}

	const CColor& getColor () const { return color; }

	void setColor (const CColor& newColor)
	{
		color = newColor;
	}

	// we accept strings which look like : '#ff3355' (rgb) and '#ff3355bb' (rgba)
	static bool dragContainerHasColor (IDataPackage* drag, CColor* color)
	{
		IDataPackage::Type type;
		const void* item;
		if (drag->getData (0, item, type) > 0 && type == IDataPackage::kText)
		{
			auto text = static_cast<UTF8StringPtr> (item);
			std::string colorString (text);
			if (colorString.length () == 7)
			{
				if (colorString[0] == '#')
				{
					if (color)
					{
						std::string rv (colorString.substr (1, 2));
						std::string gv (colorString.substr (3, 2));
						std::string bv (colorString.substr (5, 2));
						color->red = (uint8_t)strtol (rv.c_str (), nullptr, 16);
						color->green = (uint8_t)strtol (gv.c_str (), nullptr, 16);
						color->blue = (uint8_t)strtol (bv.c_str (), nullptr, 16);
						color->alpha = 255;
					}
					return true;
				}
			}
			if (colorString.length () == 9)
			{
				if (colorString[0] == '#')
				{
					if (color)
					{
						std::string rv (colorString.substr (1, 2));
						std::string gv (colorString.substr (3, 2));
						std::string bv (colorString.substr (5, 2));
						std::string av (colorString.substr (7, 2));
						color->red = (uint8_t)strtol (rv.c_str (), nullptr, 16);
						color->green = (uint8_t)strtol (gv.c_str (), nullptr, 16);
						color->blue = (uint8_t)strtol (bv.c_str (), nullptr, 16);
						color->alpha = (uint8_t)strtol (av.c_str (), nullptr, 16);
					}
					return true;
				}
			}
		}
		return false;
	}

	SharedPointer<IDropTarget> getDropTarget () override { return this; }

	bool onDrop (DragEventData data) override
	{
		CColor dragColor;
		if (dragContainerHasColor (data.drag, &dragColor))
		{
			setColor (dragColor);
			valueChanged ();
			return true;
		}
		return false;
	}
	
	DragOperation onDragEnter (DragEventData data) override
	{
		dragOperation =
		    dragContainerHasColor (data.drag, nullptr) ? DragOperation::Copy : DragOperation::None;
		return dragOperation;
	}
	
	DragOperation onDragMove (DragEventData data) override
	{
		return dragOperation;
	}

	void onDragLeave (DragEventData data) override
	{
		dragOperation = DragOperation::None;
	}
	
	CLASS_METHODS(ColorView, CControl)
protected:
	DragOperation dragOperation {DragOperation::None};
	CColor color;
	CColor checkerBoardColor1;
	CColor checkerBoardColor2;
	bool checkerBoardBack;
};

//-----------------------------------------------------------------------------
static void setupParamDisplay (CParamDisplay* display, const CColorChooserUISettings& settings)
{
	display->setFont (settings.font);
	display->setFontColor (settings.fontColor);
	display->setTransparency (true);
}

} // CColorChooserInternal

/// @endcond

//-----------------------------------------------------------------------------
bool CColorChooser::convertNormalizedToString (float value, char string[256], CParamDisplay::ValueToStringUserData* userData)
{
	snprintf (string, 255, "%.3f", value);
	return true;
}

//-----------------------------------------------------------------------------
bool CColorChooser::convertColorValueToString (float value, char string[256], CParamDisplay::ValueToStringUserData* userData)
{
	snprintf (string, 255, "%d", (int32_t)(value * 255.f));
	return true;
}

//-----------------------------------------------------------------------------
bool CColorChooser::convertAngleToString (float value, char string[256], CParamDisplay::ValueToStringUserData* userData)
{
	snprintf (string, 255, "%d%s", (int32_t)(value * 359.f), kDegreeSymbol);
	return true;
}

//-----------------------------------------------------------------------------
bool CColorChooser::convertNormalized (UTF8StringPtr string, float& output, CTextEdit::StringToValueUserData* userData)
{
	output = UTF8StringView (string).toFloat ();
	if (output < 0.f)
		output = 0.f;
	else if (output > 1.f)
		output = 1.f;
	return true;
}

//-----------------------------------------------------------------------------
bool CColorChooser::convertColorValue (UTF8StringPtr string, float& output, CTextEdit::StringToValueUserData* userData)
{
	output = UTF8StringView (string).toFloat ();
	if (output < 0.f)
		output = 0.f;
	else if (output > 255.f)
		output = 255.f;
	output /= 255.f;
	return true;
}

//-----------------------------------------------------------------------------
bool CColorChooser::convertAngle (UTF8StringPtr string, float& output, CTextEdit::StringToValueUserData* userData)
{
	output = UTF8StringView (string).toFloat ();
	if (output < 0.f)
		output = 0.f;
	else if (output > 359.f)
		output = 359.f;
	output /= 359.f;
	return true;
}

//-----------------------------------------------------------------------------
CColorChooser::CColorChooser (IColorChooserDelegate* delegate, const CColor& initialColor, const CColorChooserUISettings& settings)
: CViewContainer (CRect (0, 0, 0, 0))
, delegate (delegate)
, color (initialColor)
, redSlider (nullptr)
, greenSlider (nullptr)
, blueSlider (nullptr)
, hueSlider (nullptr)
, saturationSlider (nullptr)
, brightnessSlider (nullptr)
, alphaSlider (nullptr)
, colorView (nullptr)
{
	setTransparency (true);
	setAutosizeFlags (kAutosizeAll);

	const CCoord controlHeight = settings.font->getSize () + 2;
	const CCoord controlWidth = 150;
	const CCoord editWidth = 40;
	const CCoord labelWidth = 40;
	const CCoord xMargin = settings.margin.x;
	const CCoord yMargin = settings.margin.y;
	
	colorView = new CColorChooserInternal::ColorView (CRect (1, 1, labelWidth + xMargin + controlWidth + xMargin + editWidth, 100), initialColor, this, kColorTag, settings.checkerBoardBack, settings.checkerBoardColor1, settings.checkerBoardColor2);
	colorView->setAutosizeFlags (kAutosizeAll);
	addView (colorView);
	CRect r (colorView->getViewSize ());
	r.offset (labelWidth + xMargin, r.bottom + yMargin);
	r.setWidth (controlWidth);
	r.setHeight (controlHeight);

	redSlider = new CColorChooserInternal::Slider (r, this, kRedTag);
	redSlider->setAutosizeFlags (kAutosizeLeft|kAutosizeRight|kAutosizeBottom);
	addView (redSlider);
	
	r.offset (0, yMargin + controlHeight);
	greenSlider = new CColorChooserInternal::Slider (r, this, kGreenTag);
	greenSlider->setAutosizeFlags (kAutosizeLeft|kAutosizeRight|kAutosizeBottom);
	addView (greenSlider);

	r.offset (0, yMargin + controlHeight);
	blueSlider = new CColorChooserInternal::Slider (r, this, kBlueTag);
	blueSlider->setAutosizeFlags (kAutosizeLeft|kAutosizeRight|kAutosizeBottom);
	addView (blueSlider);

	r.offset (0, yMargin + yMargin + controlHeight);
	hueSlider = new CColorChooserInternal::Slider (r, this, kHueTag);
	hueSlider->setAutosizeFlags (kAutosizeLeft|kAutosizeRight|kAutosizeBottom);
	addView (hueSlider);

	r.offset (0, yMargin + controlHeight);
	saturationSlider = new CColorChooserInternal::Slider (r, this, kSaturationTag);
	saturationSlider->setAutosizeFlags (kAutosizeLeft|kAutosizeRight|kAutosizeBottom);
	addView (saturationSlider);

	r.offset (0, yMargin + controlHeight);
	brightnessSlider = new CColorChooserInternal::Slider (r, this, kBrightnessTag);
	brightnessSlider->setAutosizeFlags (kAutosizeLeft|kAutosizeRight|kAutosizeBottom);
	addView (brightnessSlider);

	r.offset (0, yMargin + yMargin + controlHeight);
	alphaSlider = new CColorChooserInternal::Slider (r, this, kAlphaTag);
	alphaSlider->setAutosizeFlags (kAutosizeLeft|kAutosizeRight|kAutosizeBottom);
	addView (alphaSlider);

	CRect newSize (getViewSize ());
	
	newSize.bottom = r.bottom+1;
	newSize.right = colorView->getViewSize ().right+2;

	setAutosizingEnabled (false);
	setViewSize (newSize);	
	setMouseableArea (newSize);
	setAutosizingEnabled (true);

	r = colorView->getViewSize ();
	r.offset (0, r.bottom + yMargin);
	r.setWidth (labelWidth);
	r.setHeight (controlHeight);
	auto* label = new CTextLabel (r, "Red");
	CColorChooserInternal::setupParamDisplay (label, settings);
	label->setAutosizeFlags (kAutosizeLeft|kAutosizeBottom);
	addView (label);

	r.offset (0, yMargin + controlHeight);
	label = new CTextLabel (r, "Green");
	CColorChooserInternal::setupParamDisplay (label, settings);
	label->setAutosizeFlags (kAutosizeLeft|kAutosizeBottom);
	addView (label);

	r.offset (0, yMargin + controlHeight);
	label = new CTextLabel (r, "Blue");
	CColorChooserInternal::setupParamDisplay (label, settings);
	label->setAutosizeFlags (kAutosizeLeft|kAutosizeBottom);
	addView (label);

	r.offset (0, yMargin + yMargin + controlHeight);
	label = new CTextLabel (r, "Hue");
	CColorChooserInternal::setupParamDisplay (label, settings);
	label->setAutosizeFlags (kAutosizeLeft|kAutosizeBottom);
	addView (label);

	r.offset (0, yMargin + controlHeight);
	label = new CTextLabel (r, "Sat");
	CColorChooserInternal::setupParamDisplay (label, settings);
	label->setAutosizeFlags (kAutosizeLeft|kAutosizeBottom);
	addView (label);

	r.offset (0, yMargin + controlHeight);
	label = new CTextLabel (r, "Value");
	CColorChooserInternal::setupParamDisplay (label, settings);
	label->setAutosizeFlags (kAutosizeLeft|kAutosizeBottom);
	addView (label);

	r.offset (0, yMargin + yMargin + controlHeight);
	label = new CTextLabel (r, "Alpha");
	CColorChooserInternal::setupParamDisplay (label, settings);
	label->setAutosizeFlags (kAutosizeLeft|kAutosizeBottom);
	addView (label);

	r = colorView->getViewSize ();
	r.offset (labelWidth + xMargin + controlWidth + xMargin, r.bottom + yMargin);
	r.setWidth (editWidth);
	r.setHeight (controlHeight);
	editFields[0] = new CTextEdit (r, this, kRedTag, nullptr);
	CColorChooserInternal::setupParamDisplay (editFields[0], settings);
	editFields[0]->setAutosizeFlags (kAutosizeRight|kAutosizeBottom);
	editFields[0]->setStringToValueFunction (convertColorValue);
	editFields[0]->setValueToStringFunction (convertColorValueToString);
	addView (editFields[0]);

	r.offset (0, yMargin + controlHeight);
	editFields[1] = new CTextEdit (r, this, kGreenTag, nullptr);
	CColorChooserInternal::setupParamDisplay (editFields[1], settings);
	editFields[1]->setAutosizeFlags (kAutosizeRight|kAutosizeBottom);
	editFields[1]->setStringToValueFunction (convertColorValue);
	editFields[1]->setValueToStringFunction (convertColorValueToString);
	addView (editFields[1]);

	r.offset (0, yMargin + controlHeight);
	editFields[2] = new CTextEdit (r, this, kBlueTag, nullptr);
	CColorChooserInternal::setupParamDisplay (editFields[2], settings);
	editFields[2]->setAutosizeFlags (kAutosizeRight|kAutosizeBottom);
	editFields[2]->setStringToValueFunction (convertColorValue);
	editFields[2]->setValueToStringFunction (convertColorValueToString);
	addView (editFields[2]);

	r.offset (0, yMargin + yMargin + controlHeight);
	editFields[3] = new CTextEdit (r, this, kHueTag, nullptr);
	CColorChooserInternal::setupParamDisplay (editFields[3], settings);
	editFields[3]->setAutosizeFlags (kAutosizeRight|kAutosizeBottom);
	editFields[3]->setStringToValueFunction (convertColorValue);
	editFields[3]->setValueToStringFunction (convertColorValueToString);
	addView (editFields[3]);

	r.offset (0, yMargin + controlHeight);
	editFields[4] = new CTextEdit (r, this, kSaturationTag, nullptr);
	CColorChooserInternal::setupParamDisplay (editFields[4], settings);
	editFields[4]->setAutosizeFlags (kAutosizeRight|kAutosizeBottom);
	editFields[4]->setStringToValueFunction (convertColorValue);
	editFields[4]->setValueToStringFunction (convertColorValueToString);
	addView (editFields[4]);

	r.offset (0, yMargin + controlHeight);
	editFields[5] = new CTextEdit (r, this, kBrightnessTag, nullptr);
	CColorChooserInternal::setupParamDisplay (editFields[5], settings);
	editFields[5]->setAutosizeFlags (kAutosizeRight|kAutosizeBottom);
	editFields[5]->setStringToValueFunction (convertColorValue);
	editFields[5]->setValueToStringFunction (convertColorValueToString);
	addView (editFields[5]);

	r.offset (0, yMargin + yMargin + controlHeight);
	editFields[6] = new CTextEdit (r, this, kAlphaTag, nullptr);
	CColorChooserInternal::setupParamDisplay (editFields[6], settings);
	editFields[6]->setAutosizeFlags (kAutosizeRight|kAutosizeBottom);
	editFields[6]->setStringToValueFunction (convertColorValue);
	editFields[6]->setValueToStringFunction (convertColorValueToString);
	addView (editFields[6]);

	updateState ();
}

//-----------------------------------------------------------------------------
void CColorChooser::valueChanged (CControl* control)
{
	switch (control->getTag ())
	{
		case kRedTag:
		{
			color.setNormRed (control->getValue ());
			break;
		}
		case kGreenTag:
		{
			color.setNormGreen (control->getValue ());
			break;
		}
		case kBlueTag:
		{
			color.setNormBlue (control->getValue ());
			break;
		}
		case kAlphaTag:
		{
			color.setNormAlpha (control->getValue ());
			break;
		}
		case kHueTag:
		{
			double hue, saturation, value;
			color.toHSV (hue, saturation, value);
			hue = control->getValue () * 359.;
			color.fromHSV (hue, saturation, value);
			break;
		}
		case kSaturationTag:
		{
			double hue, saturation, value;
			color.toHSV (hue, saturation, value);
			saturation = control->getValue ();
			color.fromHSV (hue, saturation, value);
			break;
		}
		case kBrightnessTag:
		{
			double hue, saturation, value;
			color.toHSV (hue, saturation, value);
			value = control->getValue ();
			color.fromHSV (hue, saturation, value);
			break;
		}
		case kColorTag:
		{
			color = colorView->getColor ();
		}
	}
	updateState ();
	if (delegate)
		delegate->colorChanged (this, color);
}

//-----------------------------------------------------------------------------
void CColorChooser::controlBeginEdit (CControl* pControl)
{
	if (delegate)
		delegate->onBeginColorChange (this);
}

//-----------------------------------------------------------------------------
void CColorChooser::controlEndEdit (CControl* pControl)
{
	if (delegate)
		delegate->onEndColorChange (this);
}

//-----------------------------------------------------------------------------
void CColorChooser::setColor (const CColor& newColor)
{
	color = newColor;
	updateState ();
}

//-----------------------------------------------------------------------------
void CColorChooser::updateState ()
{
	double hue, saturation, value;
	color.toHSV (hue, saturation, value);
	redSlider->setValue (color.normRed<float> ());
	greenSlider->setValue (color.normGreen<float> ());
	blueSlider->setValue (color.normBlue<float> ());
	alphaSlider->setValue (color.normAlpha<float> ());
	hueSlider->setValue ((float)(hue / 359.));
	saturationSlider->setValue ((float)saturation);
	brightnessSlider->setValue ((float)value);
	colorView->setColor (color);

	editFields[0]->setValue (redSlider->getValue ());
	editFields[1]->setValue (greenSlider->getValue ());
	editFields[2]->setValue (blueSlider->getValue ());
	editFields[3]->setValue (hueSlider->getValue ());
	editFields[4]->setValue (saturationSlider->getValue ());
	editFields[5]->setValue (brightnessSlider->getValue ());
	editFields[6]->setValue (alphaSlider->getValue ());

	for (int32_t i = 0; i < 7; i++)
		editFields[i]->invalid ();

	redSlider->invalid ();
	greenSlider->invalid ();
	blueSlider->invalid ();
	alphaSlider->invalid ();
	hueSlider->invalid ();
	saturationSlider->invalid ();
	brightnessSlider->invalid ();
	colorView->invalid ();
}

} // VSTGUI
