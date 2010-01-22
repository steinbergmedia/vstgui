//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins : 
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

#ifndef __ctextedit__
#define __ctextedit__

#include "ctextlabel.h"
#include "../platform/iplatformtextedit.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
// CTextEdit Declaration
//! @brief a text edit control
/// @ingroup controls
//-----------------------------------------------------------------------------
class CTextEdit : public CParamDisplay, public IPlatformTextEditCallback
{
public:
	CTextEdit (const CRect& size, CControlListener* listener, long tag, const char* txt = 0, CBitmap* background = 0, const long style = 0);
	CTextEdit (const CTextEdit& textEdit);

	//-----------------------------------------------------------------------------
	/// @name CTextEdit Methods
	//-----------------------------------------------------------------------------
	//@{
	virtual void setText (const char* txt);					///< set the text (only 256 bytes are allowed)
	virtual void getText (char* txt) const;					///< copies text to txt (make sure txt is at least 256 bytes big)
	virtual const char* getText () const { return text; }	///< read only access to text

	virtual void setTextEditConvert (void (*editConvert) (char* input, char* string));
	virtual void setTextEditConvert (void (*editConvert2) (char* input, char* string,
										void* userDta), void* userData);
	//@}

	// overrides
	virtual void setValue (float val, bool updateSubListeners = false);

	virtual	void draw (CDrawContext* pContext);
	virtual CMouseEventResult onMouseDown (CPoint& where, const long& buttons);
	virtual long onKeyDown (VstKeyCode& keyCode);

	virtual	void takeFocus ();
	virtual	void looseFocus ();

	virtual void setViewSize (CRect& newSize, bool invalid = true);
	virtual void parentSizeChanged ();

	bool bWasReturnPressed;

	CLASS_METHODS(CTextEdit, CParamDisplay)
protected:
	~CTextEdit ();

	CColor platformGetBackColor () const { return getBackColor (); }
	CColor platformGetFontColor () const { return getFontColor (); }
	CFontRef platformGetFont () const { return getFont (); }
	CHoriTxtAlign platformGetHoriTxtAlign () const { return getHoriAlign (); }
	const char* platformGetText () const { return text; }
	CRect platformGetSize () const;
	CRect platformGetVisibleSize () const;
	CPoint platformGetTextInset () const { return getTextInset (); }
	void platformLooseFocus (bool returnPressed);
	bool platformOnKeyDown (const VstKeyCode& key);

	IPlatformTextEdit* platformControl;

	char text[256];

	void (*editConvert) (char* input, char* string);
	void (*editConvert2) (char* input, char* string, void* userData);
};

} // namespace

#endif
