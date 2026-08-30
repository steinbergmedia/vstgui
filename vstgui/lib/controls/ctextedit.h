// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "ctextlabel.h"
#include "../dispatchlist.h"
#include <functional>

namespace VSTGUI {

using CTextEditStringToValueProc = bool (*) (UTF8StringPtr txt, float& result, void* userData);

//-----------------------------------------------------------------------------
// CTextEdit Declaration
//! @brief a text edit control
/// @ingroup controls
//-----------------------------------------------------------------------------
class CTextEdit : public CTextLabel
{
private:
	enum StyleEnum
	{
		StyleDoubleClick = CParamDisplay::LastStyle,
	};

	class PlatformCallbackImpl;

public:
	CTextEdit (const CRect& size, IControlListener* listener, int32_t tag,
			   UTF8StringPtr txt = nullptr, const SPtr<CBitmap>& background = {},
			   const int32_t style = 0);

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

	using StringToValueFunction =
		std::function<bool (UTF8StringPtr txt, float& result, CTextEdit& textEdit)>;

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

	void registerTextEditListener (const SPtr<ITextEditListener>& listener);
	void unregisterTextEditListener (const SPtr<ITextEditListener>& listener);
	//@}

	// overrides
	void setText (const UTF8String& txt) override;
	void valueChanged () override;
	bool setValue (float val) override;
	void setTextRotation (double angle) override { } // not supported

	void draw (CDrawContext& context) override;
	CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) override;
	void onKeyboardEvent (KeyboardEvent& event) override;

	void takeFocus () override;
	void looseFocus () override;
	bool wantsFocus () const override;

	void setViewSize (const CRect& newSize, bool invalid = true) override;
	void parentSizeChanged () override;

	bool bWasReturnPressed {false};

	PlatformTextEditPtr getPlatformTextEdit () const { return platformControl; }

protected:
	VSTGUI_SHAREDPTR_FRIEND (CTextEdit)
	~CTextEdit () noexcept override;

	void createPlatformTextEdit ();
	void updateText (const PlatformTextEditPtr& pte);

	virtual CRect platformGetSize () const;
	virtual CRect platformGetVisibleSize () const;
	virtual void platformTextDidChange ();
	void platformLooseFocus (bool returnPressed);
	void platformOnKeyboardEvent (KeyboardEvent& event);

	PlatformCallbackImpl* getPlatformTextEditCallback ();

	PlatformTextEditPtr platformControl;

	std::unique_ptr<PlatformCallbackImpl> platformCallback;

	StringToValueFunction stringToValueFunction;

	bool immediateTextChange {false};
	bool secureStyle {false};
	UTF8String placeholderString;
	DispatchList<SPtr<ITextEditListener>> textEditListeners;
};

} // VSTGUI
