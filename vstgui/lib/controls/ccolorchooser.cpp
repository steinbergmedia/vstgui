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

#include "ccolorchooser.h"
#include "cslider.h"
#include "ctextlabel.h"
#include "../cdrawcontext.h"
#include "../cframe.h"
#include <string>

namespace VSTGUI {

/// @cond ignore
namespace CColorChooserInternal {

//-----------------------------------------------------------------------------
class Slider : public CSlider
{
public:
	Slider (const CRect& size, CControlListener* listener = 0, int32_t tag = -1)
	: CSlider (size, listener, tag, 0, 0, 0, 0)
	{
		if (size.getWidth () > size.getHeight ())
			heightOfSlider = widthOfSlider = size.getHeight ();
		else
			heightOfSlider = widthOfSlider = size.getWidth ();
		CRect r (size);
		setViewSize (r, false);
	}

	void draw (CDrawContext* context)
	{
		CColor handleFillColor (kWhiteCColor);
		CColor handleFrameColor (kBlackCColor);
		CColor backgroundFillColor (kGreyCColor);
		CColor backgroundFrameColor (kBlackCColor);
		CColor bandColor (kTransparentCColor);

		CCoord backgroundFrameWidth = 1;
		CCoord handleFrameWidth = 1;

		CRect backgroundRect (0, 0, widthControl, heightControl);
		backgroundRect.offset (size.left, size.top);
		context->setDrawMode (kAntiAliasing);
		context->setFillColor (backgroundFillColor);
		context->setFrameColor (backgroundFrameColor);
		context->setLineWidth (backgroundFrameWidth);
		context->setLineStyle (kLineSolid);
		context->drawRect (backgroundRect, kDrawFilledAndStroked);
		if (style & kHorizontal)
		{
			backgroundRect.left += offsetHandle.h + widthOfSlider / 2;
			backgroundRect.right -= offsetHandle.h + widthOfSlider / 2;
			backgroundRect.top += heightControl/2 - 2;
			backgroundRect.bottom -= heightControl/2 - 2;
		}
		else
		{
			backgroundRect.left += widthControl/2 - 2;
			backgroundRect.right -= widthControl/2 - 2;
			backgroundRect.top += offsetHandle.v + heightOfSlider / 2;
			backgroundRect.bottom -= offsetHandle.v + heightOfSlider / 2;
		}
		context->setFillColor (bandColor);
		context->drawRect (backgroundRect, kDrawFilled);

		float fValue = value;
		if (style & kRight || style & kBottom)
			fValue = 1.f - value;
		
		// calc new coords of slider
		CRect rectNew;
		if (style & kHorizontal)
		{
			rectNew.top    = offsetHandle.v;
			rectNew.bottom = rectNew.top + heightOfSlider;	

			rectNew.left   = offsetHandle.h + (int32_t)(fValue * rangeHandle);
			rectNew.left   = (rectNew.left < minTmp) ? minTmp : rectNew.left;

			rectNew.right  = rectNew.left + widthOfSlider;
			rectNew.right  = (rectNew.right > maxTmp) ? maxTmp : rectNew.right;
		}
		else
		{
			rectNew.left   = offsetHandle.h;
			rectNew.right  = rectNew.left + widthOfSlider;	

			rectNew.top    = offsetHandle.v + (int32_t)(fValue * rangeHandle);
			rectNew.top    = (rectNew.top < minTmp) ? minTmp : rectNew.top;

			rectNew.bottom = rectNew.top + heightOfSlider;
			rectNew.bottom = (rectNew.bottom > maxTmp) ? maxTmp : rectNew.bottom;
		}
		rectNew.offset (size.left, size.top);

		context->setFillColor (handleFillColor);
		context->setFrameColor (handleFrameColor);
		context->setLineWidth (handleFrameWidth);
		context->drawRect (rectNew, kDrawFilledAndStroked);

		setDirty (false);
	}

};

//-----------------------------------------------------------------------------
class ColorView : public CControl
{
public:
	ColorView (const CRect& r, const CColor& initialColor, CControlListener* listener = 0, int32_t tag = -1)
	: CControl (r, listener, tag)
	, color (initialColor)
	{
	}
	
	void draw (CDrawContext* context)
	{
		context->setDrawMode (kAliasing);
		context->setFillColor (color);
		context->setFrameColor (kBlackCColor);
		context->drawRect (size, kDrawFilledAndStroked);
				
		setDirty (false);
	}

	const CColor& getColor () const { return color; }

	void setColor (const CColor& newColor)
	{
		color = newColor;
	}

	// we accept strings which look like : '#ff3355' (rgb) and '#ff3355bb' (rgba)
	static bool dragContainerHasColor (CDragContainer* dragContainer, CColor* color)
	{
		int32_t size = 0; 
		int32_t type = 0;
		void* item = dragContainer->first (size, type);
		if (type == CDragContainer::kUnicodeText)
		{
			UTF8StringPtr text = static_cast<UTF8StringPtr> (item);
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
						color->red = (uint8_t)strtol (rv.c_str (), 0, 16);
						color->green = (uint8_t)strtol (gv.c_str (), 0, 16);
						color->blue = (uint8_t)strtol (bv.c_str (), 0, 16);
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
						color->red = (uint8_t)strtol (rv.c_str (), 0, 16);
						color->green = (uint8_t)strtol (gv.c_str (), 0, 16);
						color->blue = (uint8_t)strtol (bv.c_str (), 0, 16);
						color->alpha = (uint8_t)strtol (av.c_str (), 0, 16);
					}
					return true;
				}
			}
		}
		return false;
	}

	bool onDrop (CDragContainer* drag, const CPoint& where)
	{
		CColor color;
		if (dragContainerHasColor (drag, &color))
		{
			setColor (color);
			valueChanged ();
			return true;
		}
		return false;
	}
	
	void onDragEnter (CDragContainer* drag, const CPoint& where)
	{
		if (dragContainerHasColor (drag, 0))
			getFrame ()->setCursor (kCursorCopy);
		else
			getFrame ()->setCursor (kCursorNotAllowed);
	}
	
	void onDragLeave (CDragContainer* drag, const CPoint& where)
	{
		getFrame ()->setCursor (kCursorNotAllowed);
	}
	
	CLASS_METHODS(ColorView, CControl)
protected:
	CColor color;
};

} // namespace CColorChooserInternal

/// @endcond

//-----------------------------------------------------------------------------
bool CColorChooser::convertNormalizedToString (float value, char string[256], void* userData)
{
	sprintf (string, "%.3f", value);
	return true;
}

//-----------------------------------------------------------------------------
bool CColorChooser::convertColorValueToString (float value, char string[256], void* userData)
{
	sprintf (string, "%d", (int32_t)(value*255.f));
	return true;
}

//-----------------------------------------------------------------------------
bool CColorChooser::convertAngleToString (float value, char string[256], void* userData)
{
	sprintf (string, "%d%s", (int32_t)(value*360.f), kDegreeSymbol);
	return true;
}

//-----------------------------------------------------------------------------
bool CColorChooser::convertNormalized (UTF8StringPtr string, float& output, void* userData)
{
	output = (float)strtod (string, 0);
	if (output < 0.f)
		output = 0.f;
	else if (output > 1.f)
		output = 1.f;
	return true;
}

//-----------------------------------------------------------------------------
bool CColorChooser::convertColorValue (UTF8StringPtr string, float& output, void* userData)
{
	output = (float)strtod (string, 0);
	if (output < 0.f)
		output = 0.f;
	else if (output > 255.f)
		output = 255.f;
	output /= 255.f;
	return true;
}

//-----------------------------------------------------------------------------
bool CColorChooser::convertAngle (UTF8StringPtr string, float& output, void* userData)
{
	output = (float)strtod (string, 0);
	if (output < 0.f)
		output = 0.f;
	else if (output > 360.f)
		output = 360.f;
	output /= 360.f;
	return true;
}

//-----------------------------------------------------------------------------
CColorChooser::CColorChooser (IColorChooserDelegate* delegate, const CColor& initialColor)
: CViewContainer (CRect (0, 0, 0, 0))
, delegate (delegate)
, redSlider (0)
, greenSlider (0)
, blueSlider (0)
, hueSlider (0)
, saturationSlider (0)
, brightnessSlider (0)
, alphaSlider (0)
, colorView (0)
, color (initialColor)
{
	setTransparency (true);
	setAutosizeFlags (kAutosizeAll);

	const CCoord controlHeight = 15;
	const CCoord controlWidth = 150;
	const CCoord editWidth = 40;
	const CCoord labelWidth = 40;
	const CCoord margin = 5;
	
	colorView = new CColorChooserInternal::ColorView (CRect (1, 1, labelWidth + margin + controlWidth + margin + editWidth, 100), initialColor, this, kColorTag);
	colorView->setAutosizeFlags (kAutosizeAll);
	addView (colorView);
	CRect r (colorView->getViewSize ());
	r.offset (labelWidth+margin, r.bottom + margin);
	r.setWidth (controlWidth);
	r.setHeight (controlHeight);

	redSlider = new CColorChooserInternal::Slider (r, this, kRedTag);
	redSlider->setAutosizeFlags (kAutosizeLeft|kAutosizeRight|kAutosizeBottom);
	addView (redSlider);
	
	r.offset (0, margin + controlHeight);
	greenSlider = new CColorChooserInternal::Slider (r, this, kGreenTag);
	greenSlider->setAutosizeFlags (kAutosizeLeft|kAutosizeRight|kAutosizeBottom);
	addView (greenSlider);

	r.offset (0, margin + controlHeight);
	blueSlider = new CColorChooserInternal::Slider (r, this, kBlueTag);
	blueSlider->setAutosizeFlags (kAutosizeLeft|kAutosizeRight|kAutosizeBottom);
	addView (blueSlider);

	r.offset (0, margin + margin + controlHeight);
	hueSlider = new CColorChooserInternal::Slider (r, this, kHueTag);
	hueSlider->setAutosizeFlags (kAutosizeLeft|kAutosizeRight|kAutosizeBottom);
	addView (hueSlider);

	r.offset (0, margin + controlHeight);
	saturationSlider = new CColorChooserInternal::Slider (r, this, kSaturationTag);
	saturationSlider->setAutosizeFlags (kAutosizeLeft|kAutosizeRight|kAutosizeBottom);
	addView (saturationSlider);

	r.offset (0, margin + controlHeight);
	brightnessSlider = new CColorChooserInternal::Slider (r, this, kBrightnessTag);
	brightnessSlider->setAutosizeFlags (kAutosizeLeft|kAutosizeRight|kAutosizeBottom);
	addView (brightnessSlider);

	r.offset (0, margin + margin + controlHeight);
	alphaSlider = new CColorChooserInternal::Slider (r, this, kAlphaTag);
	alphaSlider->setAutosizeFlags (kAutosizeLeft|kAutosizeRight|kAutosizeBottom);
	addView (alphaSlider);

	size.bottom = r.bottom+1;
	size.right = colorView->getViewSize ().right+2;
	
	setMouseableArea (size);

	r = colorView->getViewSize ();
	r.offset (0, r.bottom + margin);
	r.setWidth (labelWidth);
	r.setHeight (controlHeight);
	CTextLabel* label = new CTextLabel (r, "Red");
	label->setAutosizeFlags (kAutosizeLeft|kAutosizeBottom);
	label->setTransparency (true);
	addView (label);

	r.offset (0, margin + controlHeight);
	label = new CTextLabel (r, "Green");
	label->setAutosizeFlags (kAutosizeLeft|kAutosizeBottom);
	label->setTransparency (true);
	addView (label);

	r.offset (0, margin + controlHeight);
	label = new CTextLabel (r, "Blue");
	label->setAutosizeFlags (kAutosizeLeft|kAutosizeBottom);
	label->setTransparency (true);
	addView (label);

	r.offset (0, margin + margin + controlHeight);
	label = new CTextLabel (r, "Hue");
	label->setAutosizeFlags (kAutosizeLeft|kAutosizeBottom);
	label->setTransparency (true);
	addView (label);

	r.offset (0, margin + controlHeight);
	label = new CTextLabel (r, "Sat");
	label->setAutosizeFlags (kAutosizeLeft|kAutosizeBottom);
	label->setTransparency (true);
	addView (label);

	r.offset (0, margin + controlHeight);
	label = new CTextLabel (r, "Value");
	label->setAutosizeFlags (kAutosizeLeft|kAutosizeBottom);
	label->setTransparency (true);
	addView (label);

	r.offset (0, margin + margin + controlHeight);
	label = new CTextLabel (r, "Alpha");
	label->setAutosizeFlags (kAutosizeLeft|kAutosizeBottom);
	label->setTransparency (true);
	addView (label);

	r = colorView->getViewSize ();
	r.offset (labelWidth + margin + controlWidth + margin, r.bottom + margin);
	r.setWidth (editWidth);
	r.setHeight (controlHeight);
	editFields[0] = new CTextEdit (r, this, kRedTag, 0);
	editFields[0]->setAutosizeFlags (kAutosizeRight|kAutosizeBottom);
	editFields[0]->setTransparency (true);
	editFields[0]->setStringToValueProc (convertColorValue);
	editFields[0]->setValueToStringProc (convertColorValueToString);
	addView (editFields[0]);

	r.offset (0, margin + controlHeight);
	editFields[1] = new CTextEdit (r, this, kGreenTag, 0);
	editFields[1]->setAutosizeFlags (kAutosizeRight|kAutosizeBottom);
	editFields[1]->setTransparency (true);
	editFields[1]->setStringToValueProc (convertColorValue);
	editFields[1]->setValueToStringProc (convertColorValueToString);
	addView (editFields[1]);

	r.offset (0, margin + controlHeight);
	editFields[2] = new CTextEdit (r, this, kBlueTag, 0);
	editFields[2]->setAutosizeFlags (kAutosizeRight|kAutosizeBottom);
	editFields[2]->setTransparency (true);
	editFields[2]->setStringToValueProc (convertColorValue);
	editFields[2]->setValueToStringProc (convertColorValueToString);
	addView (editFields[2]);

	r.offset (0, margin + margin + controlHeight);
	editFields[3] = new CTextEdit (r, this, kHueTag, 0);
	editFields[3]->setAutosizeFlags (kAutosizeRight|kAutosizeBottom);
	editFields[3]->setTransparency (true);
	editFields[3]->setStringToValueProc (convertAngle);
	editFields[3]->setValueToStringProc (convertAngleToString);
	addView (editFields[3]);

	r.offset (0, margin + controlHeight);
	editFields[4] = new CTextEdit (r, this, kSaturationTag, 0);
	editFields[4]->setAutosizeFlags (kAutosizeRight|kAutosizeBottom);
	editFields[4]->setTransparency (true);
	editFields[4]->setStringToValueProc (convertNormalized);
	editFields[4]->setValueToStringProc (convertNormalizedToString);
	addView (editFields[4]);

	r.offset (0, margin + controlHeight);
	editFields[5] = new CTextEdit (r, this, kBrightnessTag, 0);
	editFields[5]->setAutosizeFlags (kAutosizeRight|kAutosizeBottom);
	editFields[5]->setTransparency (true);
	editFields[5]->setStringToValueProc (convertNormalized);
	editFields[5]->setValueToStringProc (convertNormalizedToString);
	addView (editFields[5]);

	r.offset (0, margin + margin + controlHeight);
	editFields[6] = new CTextEdit (r, this, kAlphaTag, 0);
	editFields[6]->setAutosizeFlags (kAutosizeRight|kAutosizeBottom);
	editFields[6]->setTransparency (true);
	editFields[6]->setStringToValueProc (convertColorValue);
	editFields[6]->setValueToStringProc (convertColorValueToString);
	addView (editFields[6]);

	updateState ();
}

//-----------------------------------------------------------------------------
CColorChooser::~CColorChooser ()
{
}

//-----------------------------------------------------------------------------
void CColorChooser::valueChanged (CControl* control)
{
	switch (control->getTag ())
	{
		case kRedTag:
		{
			color.red = (uint8_t) (control->getValue () * 255.f);
			break;
		}
		case kGreenTag:
		{
			color.green = (uint8_t) (control->getValue () * 255.f);
			break;
		}
		case kBlueTag:
		{
			color.blue = (uint8_t) (control->getValue () * 255.f);
			break;
		}
		case kAlphaTag:
		{
			color.alpha = (uint8_t) (control->getValue () * 255.f);
			break;
		}
		case kHueTag:
		{
			double hue, saturation, value;
			color.toHSV (hue, saturation, value);
			hue = control->getValue () * 360.;
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
	redSlider->setValue (color.red / 255.f);
	greenSlider->setValue (color.green / 255.f);
	blueSlider->setValue (color.blue / 255.f);
	alphaSlider->setValue (color.alpha / 255.f);
	hueSlider->setValue ((float)(hue / 360.));
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

} // namespace

