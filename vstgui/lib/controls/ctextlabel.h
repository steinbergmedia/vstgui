// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "cparamdisplay.h"
#include "itextlabellistener.h"
#include "../dispatchlist.h"
#include "../cstring.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
// CLabel Declaration
//! @brief a text label
/// @ingroup controls
//-----------------------------------------------------------------------------
class CTextLabel : public CParamDisplay
{
public:
	CTextLabel (const CRect& size, UTF8StringPtr txt = nullptr, CBitmap* background = nullptr, const int32_t style = 0);
	CTextLabel (const CTextLabel& textLabel);
	
	//-----------------------------------------------------------------------------
	/// @name CTextLabel Methods
	//-----------------------------------------------------------------------------
	//@{
	/** set text */
	virtual void setText (const UTF8String& txt);
	/** read only access to text */
	virtual const UTF8String& getText () const;

	enum TextTruncateMode {
		/** no characters will be removed */
		kTruncateNone = 0,
		/** characters will be removed from the beginning of the text */
		kTruncateHead,
		/** characters will be removed from the end of the text */
		kTruncateTail
	};
	
	/** set text truncate mode */
	virtual void setTextTruncateMode (TextTruncateMode mode);
	/** get text truncate mode */
	TextTruncateMode getTextTruncateMode () const { return textTruncateMode; }
	/** get the truncated text */
	const UTF8String& getTruncatedText () const { return truncatedText; }

	/** register a text label listener */
	void registerTextLabelListener (ITextLabelListener* listener);
	/** unregister a text label listener */
	void unregisterTextLabelListener (ITextLabelListener* listener);
	//@}

	void draw (CDrawContext* pContext) override;
	bool sizeToFit () override;
	void setViewSize (const CRect& rect, bool invalid = true) override;
	void drawStyleChanged () override;
	void valueChanged () override;

	CLASS_METHODS(CTextLabel, CParamDisplay)
protected:
	~CTextLabel () noexcept override = default;
	void freeText ();
	void calculateTruncatedText ();

#if VSTGUI_ENABLE_DEPRECATED_METHODS
	bool onWheel (const CPoint& where, const CMouseWheelAxis& axis, const float& distance, const CButtonState& buttons) override { return false; }
#endif

	TextTruncateMode textTruncateMode;
	UTF8String text;
	UTF8String truncatedText;
	using TextLabelListenerList = DispatchList<ITextLabelListener*>;
	std::unique_ptr<TextLabelListenerList> listeners;
};

//-----------------------------------------------------------------------------
/** Multi line text label
 *	@ingroup new_in_4_5
 */
class CMultiLineTextLabel : public CTextLabel
{
public:
	CMultiLineTextLabel (const CRect& size);
	CMultiLineTextLabel (const CMultiLineTextLabel&) = default;

	enum class LineLayout {
		/** clip lines overflowing the view size width */
		clip,
		/** truncate lines overflowing the view size width */
		truncate,
		/** wrap overflowing words to next line */
		wrap
	};
	void setLineLayout (LineLayout layout);
	LineLayout getLineLayout () const { return lineLayout; }

	/** automatically resize the view according to the contents (only the height)
	 *	@param state on or off
	 */
	void setAutoHeight (bool state);
	/** returns true if this view resizes itself according to the contents */
	bool getAutoHeight () const { return autoHeight; }

	/** draw the lines vertical centered
	 *	@param state on or off
	 */
	void setVerticalCentered (bool state);
	/** returns true if the view draws the lines vertically centered */
	bool getVerticalCentered () const { return verticalCentered; }

	/** return the maximum line width of all lines */
	CCoord getMaxLineWidth ();

	void drawRect (CDrawContext* pContext, const CRect& updateRect) override;
	bool sizeToFit () override;
	void setText (const UTF8String& txt) override;
	void setViewSize (const CRect& rect, bool invalid = true) override;
	void setTextTruncateMode (TextTruncateMode mode) override;
	void setValue (float val) override;
private:
	void drawStyleChanged () override;
	void calculateWrapLine  (CDrawContext *context, std::pair<UTF8String, double> &element, const IFontPainter *const &fontPainter, double lineHeight, double lineWidth, double maxWidth, const CPoint &textInset, CCoord &y);
	
	void recalculateLines (CDrawContext* context);
	void recalculateHeight ();
	
	bool autoHeight {false};
	bool verticalCentered {false};
	LineLayout lineLayout {LineLayout::clip};

	struct Line
	{
		CRect r;
		UTF8String str;
	};
	using Lines = std::vector<Line>;
	Lines lines;
};

} // VSTGUI
