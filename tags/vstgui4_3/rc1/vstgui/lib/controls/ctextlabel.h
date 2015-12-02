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
	CTextLabel (const CRect& size, UTF8StringPtr txt = 0, CBitmap* background = 0, const int32_t style = 0);
	CTextLabel (const CTextLabel& textLabel);
	
	//-----------------------------------------------------------------------------
	/// @name CTextLabel Methods
	//-----------------------------------------------------------------------------
	//@{
	virtual void setText (UTF8StringPtr txt);			///< set text
	virtual const UTF8String& getText () const;				///< read only access to text

	enum TextTruncateMode {
		kTruncateNone = 0,						///< no characters will be removed
		kTruncateHead,							///< characters will be removed from the beginning of the text
		kTruncateTail							///< characters will be removed from the end of the text
	};
	
	virtual void setTextTruncateMode (TextTruncateMode mode);					///< set text truncate mode
	TextTruncateMode getTextTruncateMode () const { return textTruncateMode; }	///< get text truncate mode
	UTF8StringPtr getTruncatedText () const { return truncatedText; }			///< get the truncated text
	//@}

	static IdStringPtr kMsgTruncatedTextChanged;								///< message which is send to dependent objects when the truncated text changes
	
	virtual	void draw (CDrawContext* pContext) VSTGUI_OVERRIDE_VMETHOD;
	virtual bool sizeToFit () VSTGUI_OVERRIDE_VMETHOD;
	virtual void setViewSize (const CRect& rect, bool invalid = true) VSTGUI_OVERRIDE_VMETHOD;
	virtual void drawStyleChanged () VSTGUI_OVERRIDE_VMETHOD;

	CLASS_METHODS(CTextLabel, CParamDisplay)
protected:
	~CTextLabel ();
	void freeText ();
	void calculateTruncatedText ();

	bool onWheel (const CPoint& where, const float& distance, const CButtonState& buttons) VSTGUI_OVERRIDE_VMETHOD { return false; }
	bool onWheel (const CPoint& where, const CMouseWheelAxis& axis, const float& distance, const CButtonState& buttons) VSTGUI_OVERRIDE_VMETHOD { return false; }

	TextTruncateMode textTruncateMode;
	UTF8String text;
	UTF8String truncatedText;
};

} // namespace

#endif
