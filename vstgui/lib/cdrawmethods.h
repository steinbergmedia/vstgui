// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "vstguifwd.h"
#include "cdrawdefs.h"
#include "cfont.h"
#include "cpoint.h"

namespace VSTGUI {

namespace CDrawMethods {

//-----------------------------------------------------------------------------
enum IconPosition : uint16_t {
	/** icon left, text centered in the area next to the icon*/
	kIconLeft = 0,
	/** icon centered above the text, text centered */
	kIconCenterAbove,
	/** icon centered below the text, text centered */
	kIconCenterBelow,
	/** icon right, text centered in the area next to the icon */
	kIconRight
};

//-----------------------------------------------------------------------------
enum TextTruncateMode : uint16_t {
	kTextTruncateNone = 0,
	kTextTruncateHead,
	kTextTruncateTail
};

//-----------------------------------------------------------------------------
enum CreateTextTruncateFlags : uint16_t {
	/** return an empty string if the truncated text is only the placeholder string */
	kReturnEmptyIfTruncationIsPlaceholderOnly = 1 << 0,
};

//-----------------------------------------------------------------------------
/** create a truncated string
 *
 *	@param mode			truncation mode
 *	@param text			text string
 *	@param font			font
 *	@param maxWidth		maximum width
 *	@param textInset	text inset
 *	@param flags		flags see CreateTextTruncateFlags
 *	@return				truncated text or original text if no truncation needed
 */
UTF8String createTruncatedText (TextTruncateMode mode, const UTF8String& text, CFontRef font,
                                CCoord maxWidth, const CPoint& textInset = CPoint (0, 0),
                                uint32_t flags = 0);

//-----------------------------------------------------------------------------
/** draws an icon and a string into a rectangle
 *
 *  @param context       	draw context
 *  @param iconToDraw    	icon to draw
 *  @param iconPosition  	position of the icon
 *  @param textAlignment 	alignment of the string
 *  @param textIconMargin	margin of the string
 *  @param drawRect      	draw rectangle
 *  @param title         	string
 *  @param font          	font
 *  @param textColor     	font color
 *	@param truncateMode		truncation mode
 */
void drawIconAndText (CDrawContext* context, CBitmap* iconToDraw, IconPosition iconPosition,
                      CHoriTxtAlign textAlignment, CCoord textIconMargin, CRect drawRect,
                      const UTF8String& title, CFontRef font, CColor textColor,
                      TextTruncateMode truncateMode = kTextTruncateNone);
}} // namespaces
