//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 4.2
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2013, Steinberg Media Technologies, All Rights Reserved
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

#ifndef __ctextedit__
#define __ctextedit__

#include "ctextlabel.h"
#include "../platform/iplatformtextedit.h"

namespace VSTGUI {
class CTextEdit;

typedef bool (*CTextEditStringToValueProc) (UTF8StringPtr txt, float& result, void* userData);

//-----------------------------------------------------------------------------
// CTextEdit Declaration
//! @brief a text edit control
/// @ingroup controls
//-----------------------------------------------------------------------------
class CTextEdit : public CTextLabel, public IPlatformTextEditCallback
{
public:
	CTextEdit (const CRect& size, CControlListener* listener, int32_t tag, UTF8StringPtr txt = 0, CBitmap* background = 0, const int32_t style = 0);
	CTextEdit (const CTextEdit& textEdit);

	//-----------------------------------------------------------------------------
	/// @name CTextEdit Methods
	//-----------------------------------------------------------------------------
	//@{
	virtual void setStringToValueProc (CTextEditStringToValueProc proc, void* userData = 0);
	
	virtual void setImmediateTextChange (bool state);	///< enable/disable immediate text change behaviour.
	bool getImmediateTextChange () const { return immediateTextChange; }	///< get immediate text change behaviour
	//@}

	// overrides
	virtual void setText (UTF8StringPtr txt) VSTGUI_OVERRIDE_VMETHOD;
	virtual void setValue (float val) VSTGUI_OVERRIDE_VMETHOD;

	virtual	void draw (CDrawContext* pContext) VSTGUI_OVERRIDE_VMETHOD;
	virtual CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) VSTGUI_OVERRIDE_VMETHOD;
	virtual int32_t onKeyDown (VstKeyCode& keyCode) VSTGUI_OVERRIDE_VMETHOD;

	virtual	void takeFocus () VSTGUI_OVERRIDE_VMETHOD;
	virtual	void looseFocus () VSTGUI_OVERRIDE_VMETHOD;

	virtual void setViewSize (const CRect& newSize, bool invalid = true) VSTGUI_OVERRIDE_VMETHOD;
	virtual void parentSizeChanged () VSTGUI_OVERRIDE_VMETHOD;

	bool bWasReturnPressed;

	IPlatformTextEdit* getPlatformTextEdit () const { return platformControl; }

	CLASS_METHODS(CTextEdit, CParamDisplay)
protected:
	~CTextEdit ();

	void updateText (IPlatformTextEdit* pte);

	CColor platformGetBackColor () const { return getBackColor (); }
	CColor platformGetFontColor () const { return getFontColor (); }
	CFontRef platformGetFont () const { return getFont (); }
	CHoriTxtAlign platformGetHoriTxtAlign () const { return getHoriAlign (); }
	UTF8StringPtr platformGetText () const { return text; }
	CRect platformGetSize () const;
	CRect platformGetVisibleSize () const;
	CPoint platformGetTextInset () const { return getTextInset (); }
	void platformLooseFocus (bool returnPressed);
	bool platformOnKeyDown (const VstKeyCode& key);
	void platformTextDidChange ();

	IPlatformTextEdit* platformControl;

	CTextEditStringToValueProc textToValue;
	void* textToValueUserData;
	
	bool immediateTextChange;
};

} // namespace

#endif
