// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "ctextlabel.h"
#include "../dispatchlist.h"
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
private:
	enum StyleEnum
	{
		StyleDoubleClick = CParamDisplay::LastStyle,
	};
public:
	using PlatformTextEditPtr = SharedPointer<IPlatformTextEdit>;

	CTextEdit (const CRect& size, IControlListener* listener, int32_t tag, UTF8StringPtr txt = nullptr, CBitmap* background = nullptr, const int32_t style = 0);
	CTextEdit (const CTextEdit& textEdit);

	enum Style
	{
		kDoubleClickStyle = 1 << StyleDoubleClick,
	};

	bool isDoubleClickStyle () const { return hasBit (getStyle (), kDoubleClickStyle); }

	//-----------------------------------------------------------------------------
	/// @name CTextEdit Methods
	//-----------------------------------------------------------------------------
	//@{
	using StringToValueUserData = CTextEdit;

	using StringToValueFunction = std::function<bool(UTF8StringPtr txt, float& result, CTextEdit* textEdit)>;
	
	void setStringToValueFunction (const StringToValueFunction& stringToValueFunc);
	void setStringToValueFunction (StringToValueFunction&& stringToValueFunc);
	
	/** enable/disable immediate text change behaviour */
	virtual void setImmediateTextChange (bool state);
	/** get immediate text change behaviour */
	bool getImmediateTextChange () const { return immediateTextChange; }

	/** enable/disable secure style */
	void setSecureStyle (bool state);
	/** get secure style */
	bool getSecureStyle () const;
	
	virtual void setPlaceholderString (const UTF8String& str);
	const UTF8String& getPlaceholderString () const { return placeholderString; }

	void registerTextEditListener (ITextEditListener* listener);
	void unregisterTextEditListener (ITextEditListener* listener);
	//@}

	// overrides
	void setText (const UTF8String& txt) override;
	void valueChanged () override;
	void setValue (float val) override;
	void setTextRotation (double angle) override { } // not supported

	void draw (CDrawContext* pContext) override;
	CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) override;
	void onKeyboardEvent (KeyboardEvent& event) override;

	void takeFocus () override;
	void looseFocus () override;
	bool wantsFocus () const override;

	void setViewSize (const CRect& newSize, bool invalid = true) override;
	void parentSizeChanged () override;

	bool bWasReturnPressed {false};

	PlatformTextEditPtr getPlatformTextEdit () const { return platformControl; }

	CLASS_METHODS(CTextEdit, CParamDisplay)
protected:
	~CTextEdit () noexcept override;

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
	void platformOnKeyboardEvent (KeyboardEvent& event) override;
	void platformTextDidChange () override;
	bool platformIsSecureTextEdit () override;

	PlatformTextEditPtr platformControl;

	StringToValueFunction stringToValueFunction;

	bool immediateTextChange {false};
	bool secureStyle {false};
	mutable SharedPointer<CFontDesc> platformFont;
	UTF8String placeholderString;
	DispatchList<ITextEditListener*> textEditListeners;
};

} // VSTGUI
