//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework not only for VST plugins
//
// Version 4.3
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2015, Steinberg Media Technologies, All Rights Reserved
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

#ifndef __csegmentbutton__
#define __csegmentbutton__

#include "ccontrol.h"
#include "../cdrawmethods.h"
#include "../cbitmap.h"
#include "../cgradient.h"
#include "../cstring.h"
#include "../ccolor.h"
#include <vector>
#include <limits>

namespace VSTGUI {

//-----------------------------------------------------------------------------
///	@brief Control which draws a segmented button
///	@ingroup new_in_4_3
//-----------------------------------------------------------------------------
class CSegmentButton : public CControl
{
public:
	enum Style {
		kHorizontal,
		kVertical
	};
	
	struct Segment {
		mutable UTF8String name;
		mutable SharedPointer<CBitmap> icon;
		mutable SharedPointer<CBitmap> iconHighlighted;
		mutable SharedPointer<CBitmap> background;
		mutable SharedPointer<CBitmap> backgroundHighlighted;
		mutable CDrawMethods::IconPosition iconPosition;

		CRect rect;
	};
	typedef std::vector<Segment> Segments;
	static uint32_t kPushBack;

	CSegmentButton (const CRect& size, IControlListener* listener = 0, int32_t tag = -1);

	//-----------------------------------------------------------------------------
	/// @name Segment Methods
	//-----------------------------------------------------------------------------
	//@{
	void addSegment (Segment segment, uint32_t index = kPushBack);
	void removeSegment (uint32_t index);
	void removeAllSegments ();
	const Segments& getSegments () const { return segments; }

	void setSelectedSegment (uint32_t index);
	uint32_t getSelectedSegment () const;
	//@}

	//-----------------------------------------------------------------------------
	/// @name CSegmentButton Style Methods
	//-----------------------------------------------------------------------------
	//@{
	void setStyle (Style newStyle);
	Style getStyle () const { return style; }

	void setTextTruncateMode (CDrawMethods::TextTruncateMode mode);
	CDrawMethods::TextTruncateMode getTextTruncateMode () const { return textTruncateMode; }

	void setGradient (CGradient* newGradient);
	CGradient* getGradient () const { return gradient; }
	
	void setGradientHighlighted (CGradient* newGradient);
	CGradient* getGradientHighlighted () const { return gradientHighlighted; }
	
	void setRoundRadius (CCoord newRoundRadius);
	CCoord getRoundRadius () const { return roundRadius; }

	void setFont (CFontRef font);
	CFontRef getFont () const { return font; }

	void setTextAlignment (CHoriTxtAlign alignment);
	CHoriTxtAlign getTextAlignment () const { return textAlignment; }

	void setTextMargin (CCoord newMargin);
	CCoord getTextMargin () const { return textMargin; }

	void setTextColor (CColor newColor);
	CColor getTextColor () const { return textColor; }
	
	void setTextColorHighlighted (CColor newColor);
	CColor getTextColorHighlighted () const { return textColorHighlighted; }
	
	void setFrameColor (CColor newColor);
	CColor getFrameColor () const { return frameColor; }
	
	void setFrameWidth (CCoord newWidth);
	CCoord getFrameWidth () const { return frameWidth; }
	//@}
	
	// overrides
	bool attached (CView *parent) VSTGUI_OVERRIDE_VMETHOD;
	void setViewSize (const CRect& rect, bool invalid = true) VSTGUI_OVERRIDE_VMETHOD;
	CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) VSTGUI_OVERRIDE_VMETHOD;
	int32_t onKeyDown (VstKeyCode& keyCode) VSTGUI_OVERRIDE_VMETHOD;
	void draw (CDrawContext* pContext) VSTGUI_OVERRIDE_VMETHOD;
	void drawRect (CDrawContext* pContext, const CRect& dirtyRect) VSTGUI_OVERRIDE_VMETHOD;
	bool drawFocusOnTop () VSTGUI_OVERRIDE_VMETHOD;
	bool getFocusPath (CGraphicsPath& outPath) VSTGUI_OVERRIDE_VMETHOD;

	CLASS_METHODS(CSegmentButton, CControl)
private:
	void updateSegmentSizes ();
	uint32_t getSegmentIndex (float value) const;

	Segments segments;
	SharedPointer<CGradient> gradient;
	SharedPointer<CGradient> gradientHighlighted;
	SharedPointer<CFontDesc> font;
	CColor textColor;
	CColor textColorHighlighted;
	CColor frameColor;
	CHoriTxtAlign textAlignment;
	CCoord textMargin;
	CCoord roundRadius;
	CCoord frameWidth;
	Style style;
	CDrawMethods::TextTruncateMode textTruncateMode;
};

} // namespace

#endif // __csegmentbutton__
