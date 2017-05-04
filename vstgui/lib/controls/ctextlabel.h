// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __ctextlabel__
#define __ctextlabel__

#include "cparamdisplay.h"
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
	virtual void setText (const UTF8String& txt);			///< set text
	virtual const UTF8String& getText () const;				///< read only access to text

	enum TextTruncateMode {
		kTruncateNone = 0,						///< no characters will be removed
		kTruncateHead,							///< characters will be removed from the beginning of the text
		kTruncateTail							///< characters will be removed from the end of the text
	};
	
	virtual void setTextTruncateMode (TextTruncateMode mode);					///< set text truncate mode
	TextTruncateMode getTextTruncateMode () const { return textTruncateMode; }	///< get text truncate mode
	const UTF8String& getTruncatedText () const { return truncatedText; }		///< get the truncated text
	//@}

	static IdStringPtr kMsgTruncatedTextChanged;								///< message which is send to dependent objects when the truncated text changes
	
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

	bool onWheel (const CPoint& where, const float& distance, const CButtonState& buttons) override { return false; }
	bool onWheel (const CPoint& where, const CMouseWheelAxis& axis, const float& distance, const CButtonState& buttons) override { return false; }

	TextTruncateMode textTruncateMode;
	UTF8String text;
	UTF8String truncatedText;
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
		clip, ///< clip lines overflowing the view size width
		truncate, ///< truncate lines overflowing the view size width
		wrap ///< wrap overflowing words to next line
	};
	void setLineLayout (LineLayout layout);
	LineLayout getLineLayout () const { return lineLayout; }

	/** automatically resize the view according to the contents (only the height) 
	 *	@param state on or off
	 */
	void setAutoHeight (bool state);
	/** returns true if this view resizes itself according to the contents */
	bool getAutoHeight () const { return autoHeight; }

	/** return the maximum line width of all lines */
	CCoord getMaxLineWidth ();

	void drawRect (CDrawContext* pContext, const CRect& updateRect) override;
	bool sizeToFit () override;
	void setText (const UTF8String& txt) override;
	void setViewSize (const CRect& rect, bool invalid = true) override;
	void setTextTruncateMode (TextTruncateMode mode) override;
private:
	void drawStyleChanged () override;
	void recalculateLines (CDrawContext* context);
	void recalculateHeight ();
	
	bool autoHeight {false};
	LineLayout lineLayout {LineLayout::clip};

	struct Line
	{
		CRect r;
		UTF8String str;
	};
	using Lines = std::vector<Line>;
	Lines lines;
};

} // namespace

#endif
