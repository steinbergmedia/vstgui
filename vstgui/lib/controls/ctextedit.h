// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

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

	void setSecureStyle (bool state);	///< enable/disable secure style
	bool getSecureStyle () const;		///< get secure style
	
	virtual void setPlaceholderString (const UTF8String& str);
	const UTF8String& getPlaceholderString () const { return placeholderString; }
	//@}

	// overrides
	void setText (const UTF8String& txt) override;
	void valueChanged () override;
	void setValue (float val) override;
	void setTextRotation (double angle) override { return; } ///< not supported

	void draw (CDrawContext* pContext) override;
	CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) override;
	int32_t onKeyDown (VstKeyCode& keyCode) override;

	void takeFocus () override;
	void looseFocus () override;
	bool wantsFocus () const override;		///< check if view supports focus

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
	bool platformOnKeyDown (const VstKeyCode& key) override;
	void platformTextDidChange () override;
	bool platformIsSecureTextEdit () override;

	PlatformTextEditPtr platformControl;

	StringToValueFunction stringToValueFunction;

	bool immediateTextChange {false};
	bool secureStyle {false};
	mutable SharedPointer<CFontDesc> platformFont;
	UTF8String placeholderString;
};

} // namespace

#endif
