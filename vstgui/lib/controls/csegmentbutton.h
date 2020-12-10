// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

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
	enum class Style {
		/** horizontally layouted segments */
		kHorizontal,
		/** vertically layouted segments */
		kVertical,
		/** horizontally inverse layouted segments */
		kHorizontalInverse,
		/** vertically inverse layouted segments */
		kVerticalInverse,
	};

	enum class SelectionMode
	{
		/** a single segment is selected at any time */
		kSingle,
		/** a single segment is selected at any time, when a segment is clicked which is already
		   selected, the next segment is selected */
		kSingleToggle,
		/** multiple segments may be selected */
		kMultiple,
	};

	struct Segment {
		mutable UTF8String name;
		mutable SharedPointer<CBitmap> icon;
		mutable SharedPointer<CBitmap> iconHighlighted;
		mutable SharedPointer<CBitmap> background;
		mutable SharedPointer<CBitmap> backgroundHighlighted;
		mutable CDrawMethods::IconPosition iconPosition;

		CRect rect;
		bool selected {false};
	};
	using Segments = std::vector<Segment>;
	static constexpr uint32_t kPushBack = (std::numeric_limits<uint32_t>::max) ();

	CSegmentButton (const CRect& size, IControlListener* listener = nullptr, int32_t tag = -1);

	//-----------------------------------------------------------------------------
	/// @name Segment Methods
	//-----------------------------------------------------------------------------
	//@{
	bool addSegment (const Segment& segment, uint32_t index = kPushBack);
	bool addSegment (Segment&& segment, uint32_t index = kPushBack);
	void removeSegment (uint32_t index);
	void removeAllSegments ();
	const Segments& getSegments () const { return segments; }

	/** set the selected segment in single selection mode */
	void setSelectedSegment (uint32_t index);
	/** get the selected segment in single selection mode */
	uint32_t getSelectedSegment () const;
	
	/** set selection state for a segment in multiple selection mode */
	void selectSegment (uint32_t index, bool state);
	/** get selection state for a segment in multiple selection mode */
	bool isSegmentSelected (uint32_t index) const;
	//@}

	//-----------------------------------------------------------------------------
	/// @name CSegmentButton Style Methods
	//-----------------------------------------------------------------------------
	//@{
	void setStyle (Style newStyle);
	Style getStyle () const { return style; }

	void setSelectionMode (SelectionMode mode);
	SelectionMode getSelectionMode () const { return selectionMode; }

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
	void valueChanged () override;

	static bool isHorizontalStyle (Style style)
	{
		return style == Style::kHorizontal || style == Style::kHorizontalInverse;
	}

	static bool isVerticalStyle (Style style)
	{
		return style == Style::kVertical || style == Style::kVerticalInverse;
	}

	static bool isInverseStyle (Style style)
	{
		return style == Style::kHorizontalInverse || style == Style::kVerticalInverse;
	}

	CLASS_METHODS (CSegmentButton, CControl)
private:
	bool canAddOneMoreSegment () const;
	void updateSegmentSizes ();
	void verifySelections ();
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
	Style style {Style::kHorizontal};
	SelectionMode selectionMode {SelectionMode::kSingle};
	CDrawMethods::TextTruncateMode textTruncateMode {CDrawMethods::kTextTruncateNone};
};

} // VSTGUI
