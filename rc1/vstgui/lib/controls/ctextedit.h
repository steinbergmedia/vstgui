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

#ifndef __ctextedit__
#define __ctextedit__

#include "ctextlabel.h"
#include "../platform/iplatformtextedit.h"
#if VSTGUI_HAS_FUNCTIONAL
#include <functional>
#endif

namespace VSTGUI {

typedef bool (*CTextEditStringToValueProc) (UTF8StringPtr txt, float& result, void* userData);

//-----------------------------------------------------------------------------
// CTextEdit Declaration
//! @brief a text edit control
/// @ingroup controls
//-----------------------------------------------------------------------------
class CTextEdit : public CTextLabel, public IPlatformTextEditCallback
{
public:
	CTextEdit (const CRect& size, IControlListener* listener, int32_t tag, UTF8StringPtr txt = 0, CBitmap* background = 0, const int32_t style = 0);
	CTextEdit (const CTextEdit& textEdit);

	//-----------------------------------------------------------------------------
	/// @name CTextEdit Methods
	//-----------------------------------------------------------------------------
	//@{
#if VSTGUI_HAS_FUNCTIONAL
	typedef CTextEdit		StringToValueUserData;
#else
	typedef void			StringToValueUserData;
#endif

	VSTGUI_DEPRECATED_FUNCTIONAL(virtual void setStringToValueProc (CTextEditStringToValueProc proc, void* userData = 0);) ///< deprecated use setStringToValueFunction instead if you use c++11
#if VSTGUI_HAS_FUNCTIONAL
	typedef std::function<bool(UTF8StringPtr txt, float& result, CTextEdit* textEdit)> StringToValueFunction;
	
	void setStringToValueFunction (const StringToValueFunction& stringToValueFunc);
	void setStringToValueFunction (StringToValueFunction&& stringToValueFunc);
#endif
	
	virtual void setImmediateTextChange (bool state);	///< enable/disable immediate text change behaviour.
	bool getImmediateTextChange () const { return immediateTextChange; }	///< get immediate text change behaviour
	//@}

	// overrides
	virtual void setText (UTF8StringPtr txt) VSTGUI_OVERRIDE_VMETHOD;
	virtual void setValue (float val) VSTGUI_OVERRIDE_VMETHOD;
	virtual void setTextRotation (double angle) VSTGUI_OVERRIDE_VMETHOD { return; } ///< not supported

	virtual	void draw (CDrawContext* pContext) VSTGUI_OVERRIDE_VMETHOD;
	virtual CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) VSTGUI_OVERRIDE_VMETHOD;
	virtual int32_t onKeyDown (VstKeyCode& keyCode) VSTGUI_OVERRIDE_VMETHOD;

	virtual	void takeFocus () VSTGUI_OVERRIDE_VMETHOD;
	virtual	void looseFocus () VSTGUI_OVERRIDE_VMETHOD;
	virtual bool wantsFocus () const VSTGUI_OVERRIDE_VMETHOD;		///< check if view supports focus

	virtual void setViewSize (const CRect& newSize, bool invalid = true) VSTGUI_OVERRIDE_VMETHOD;
	virtual void parentSizeChanged () VSTGUI_OVERRIDE_VMETHOD;

	bool bWasReturnPressed;

	IPlatformTextEdit* getPlatformTextEdit () const { return platformControl; }

	CLASS_METHODS(CTextEdit, CParamDisplay)
protected:
	~CTextEdit ();

	void createPlatformTextEdit ();
	void updateText (IPlatformTextEdit* pte);

	CColor platformGetBackColor () const VSTGUI_OVERRIDE_VMETHOD { return getBackColor (); }
	CColor platformGetFontColor () const VSTGUI_OVERRIDE_VMETHOD { return getFontColor (); }
	CFontRef platformGetFont () const VSTGUI_OVERRIDE_VMETHOD;
	CHoriTxtAlign platformGetHoriTxtAlign () const VSTGUI_OVERRIDE_VMETHOD { return getHoriAlign (); }
	UTF8StringPtr platformGetText () const VSTGUI_OVERRIDE_VMETHOD { return text; }
	CRect platformGetSize () const VSTGUI_OVERRIDE_VMETHOD;
	CRect platformGetVisibleSize () const VSTGUI_OVERRIDE_VMETHOD;
	CPoint platformGetTextInset () const VSTGUI_OVERRIDE_VMETHOD { return getTextInset (); }
	void platformLooseFocus (bool returnPressed) VSTGUI_OVERRIDE_VMETHOD;
	bool platformOnKeyDown (const VstKeyCode& key) VSTGUI_OVERRIDE_VMETHOD;
	void platformTextDidChange () VSTGUI_OVERRIDE_VMETHOD;

	IPlatformTextEdit* platformControl;

#if VSTGUI_HAS_FUNCTIONAL
	StringToValueFunction stringToValueFunction;
#else
	CTextEditStringToValueProc textToValue;
	void* textToValueUserData;
#endif

	bool immediateTextChange;
	mutable SharedPointer<CFontDesc> platformFont;
};

} // namespace

#endif
