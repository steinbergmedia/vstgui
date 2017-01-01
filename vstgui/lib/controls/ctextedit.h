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
#include <functional>

namespace VSTGUI {

using CTextEditStringToValueProc = bool (*) (UTF8StringPtr txt, float& result, void* userData);

//-----------------------------------------------------------------------------
// CTextEdit Declaration
//! @brief a text edit control
/// @ingroup controls
//-----------------------------------------------------------------------------
class CTextEdit : public CTextLabel, public IPlatformTextEditCallback
{
public:
	using PlatformTextEditPtr = SharedPointer<IPlatformTextEdit>;

	CTextEdit (const CRect& size, IControlListener* listener, int32_t tag, UTF8StringPtr txt = nullptr, CBitmap* background = nullptr, const int32_t style = 0);
	CTextEdit (const CTextEdit& textEdit);

	//-----------------------------------------------------------------------------
	/// @name CTextEdit Methods
	//-----------------------------------------------------------------------------
	//@{
	using StringToValueUserData = CTextEdit;

	using StringToValueFunction = std::function<bool(UTF8StringPtr txt, float& result, CTextEdit* textEdit)>;
	
	void setStringToValueFunction (const StringToValueFunction& stringToValueFunc);
	void setStringToValueFunction (StringToValueFunction&& stringToValueFunc);
	
	virtual void setImmediateTextChange (bool state);	///< enable/disable immediate text change behaviour.
	bool getImmediateTextChange () const { return immediateTextChange; }	///< get immediate text change behaviour

	virtual void setPlaceholderString (const UTF8String& str);
	const UTF8String& getPlaceholderString () const { return placeholderString; }
	//@}

	// overrides
	virtual void setText (const UTF8String& txt) override;
	virtual void setValue (float val) override;
	virtual void setTextRotation (double angle) override { return; } ///< not supported

	virtual	void draw (CDrawContext* pContext) override;
	virtual CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) override;
	virtual int32_t onKeyDown (VstKeyCode& keyCode) override;

	virtual	void takeFocus () override;
	virtual	void looseFocus () override;
	virtual bool wantsFocus () const override;		///< check if view supports focus

	virtual void setViewSize (const CRect& newSize, bool invalid = true) override;
	virtual void parentSizeChanged () override;

	bool bWasReturnPressed;

	PlatformTextEditPtr getPlatformTextEdit () const { return platformControl; }

	CLASS_METHODS(CTextEdit, CParamDisplay)
protected:
	~CTextEdit () noexcept;

	void createPlatformTextEdit ();
	void updateText (IPlatformTextEdit* pte);

	CColor platformGetBackColor () const override { return getBackColor (); }
	CColor platformGetFontColor () const override { return getFontColor (); }
	CFontRef platformGetFont () const override;
	CHoriTxtAlign platformGetHoriTxtAlign () const override { return getHoriAlign (); }
	const UTF8String& platformGetText () const override { return text; }
	const UTF8String& platformGetPlaceholderText () const override { return placeholderString; }
	CRect platformGetSize () const override;
	CRect platformGetVisibleSize () const override;
	CPoint platformGetTextInset () const override { return getTextInset (); }
	void platformLooseFocus (bool returnPressed) override;
	bool platformOnKeyDown (const VstKeyCode& key) override;
	void platformTextDidChange () override;

	PlatformTextEditPtr platformControl;

	StringToValueFunction stringToValueFunction;

	bool immediateTextChange;
	mutable SharedPointer<CFontDesc> platformFont;
	UTF8String placeholderString;
};

} // namespace

#endif
