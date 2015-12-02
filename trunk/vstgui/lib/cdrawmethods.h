//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
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

#ifndef __cdrawmethods__
#define __cdrawmethods__

#include "vstguifwd.h"
#include "cdrawdefs.h"
#include "cfont.h"
#include "cpoint.h"

namespace VSTGUI {

namespace CDrawMethods {

//-----------------------------------------------------------------------------
enum IconPosition {
	kIconLeft,			///< icon left, text centered in the area next to the icon
	kIconCenterAbove,	///< icon centered above the text, text centered
	kIconCenterBelow,	///< icon centered below the text, text centered
	kIconRight			///< icon right, text centered in the area next to the icon
};

//-----------------------------------------------------------------------------
enum TextTruncateMode {
	kTextTruncateNone,
	kTextTruncateHead,
	kTextTruncateTail
};

//-----------------------------------------------------------------------------
enum CreateTextTruncateFlags {
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
UTF8String createTruncatedText (TextTruncateMode mode, const UTF8String& text, CFontRef font, CCoord maxWidth, const CPoint& textInset = CPoint (0, 0), uint32_t flags = 0);

//-----------------------------------------------------------------------------
/** draws an icon and a string into a rectangle
 *
 *  @param context       	draw context
 *  @param iconToDraw    	icon to draw
 *  @param iconPosition  	position of the icon
 *  @param textAlignment 	alignment of the string
 *  @param textIconMargin    margin of the string
 *  @param drawRect      	draw rectangle
 *  @param title         	string
 *  @param font          	font
 *  @param textColor     	font color
 */
void drawIconAndText (CDrawContext* context, CBitmap* iconToDraw, IconPosition iconPosition, CHoriTxtAlign textAlignment, CCoord textIconMargin, CRect drawRect, const UTF8String& title, CFontRef font, CColor textColor, TextTruncateMode truncateMode = kTextTruncateNone);
	
}} // namespaces

#endif // __cdrawmethods__
