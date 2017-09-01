// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

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
	using Segments = std::vector<Segment>;
	static uint32_t kPushBack;

	CSegmentButton (const CRect& size, IControlListener* listener = nullptr, int32_t tag = -1);

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
	bool attached (CView *parent) override;
	void setViewSize (const CRect& rect, bool invalid = true) override;
	CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) override;
	int32_t onKeyDown (VstKeyCode& keyCode) override;
	void draw (CDrawContext* pContext) override;
	void drawRect (CDrawContext* pContext, const CRect& dirtyRect) override;
	bool drawFocusOnTop () override;
	bool getFocusPath (CGraphicsPath& outPath) override;

	CLASS_METHODS(CSegmentButton, CControl)
private:
	void updateSegmentSizes ();
	uint32_t getSegmentIndex (float value) const;

	Segments segments;
	SharedPointer<CGradient> gradient;
	SharedPointer<CGradient> gradientHighlighted;
	SharedPointer<CFontDesc> font;
	CColor textColor {kBlackCColor};
	CColor textColorHighlighted {kWhiteCColor};
	CColor frameColor {kBlackCColor};
	CHoriTxtAlign textAlignment {kCenterText};
	CCoord textMargin {0.};
	CCoord roundRadius {5.};
	CCoord frameWidth {1.};
	Style style {kHorizontal};
	CDrawMethods::TextTruncateMode textTruncateMode {CDrawMethods::kTextTruncateNone};
};

} // namespace

#endif // __csegmentbutton__
