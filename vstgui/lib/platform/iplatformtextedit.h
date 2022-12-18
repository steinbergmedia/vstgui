// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

/// @cond ignore

#include "../cfont.h"
#include "../ccolor.h"
#include "../crect.h"
#include "../cdrawdefs.h"

struct VstKeyCode;

namespace VSTGUI {

//-----------------------------------------------------------------------------
class IPlatformTextEditCallback
{
public:
	virtual CColor platformGetBackColor () const = 0;
	virtual CColor platformGetFontColor () const = 0;
	virtual CFontRef platformGetFont () const = 0;
	virtual CHoriTxtAlign platformGetHoriTxtAlign () const = 0; 
	virtual const UTF8String& platformGetText () const = 0;
	virtual const UTF8String& platformGetPlaceholderText () const = 0;
	virtual CRect platformGetSize () const = 0;
	virtual CRect platformGetVisibleSize () const = 0;
	virtual CPoint platformGetTextInset () const = 0;
	virtual void platformLooseFocus (bool returnPressed) = 0;
	virtual void platformOnKeyboardEvent (KeyboardEvent& event) = 0;
	virtual void platformTextDidChange () = 0;
	virtual bool platformIsSecureTextEdit () = 0;

//------------------------------------------------------------------------------------
};

//-----------------------------------------------------------------------------
class IPlatformTextEdit : public AtomicReferenceCounted
{
public:
	virtual UTF8String getText () = 0;
	virtual bool setText (const UTF8String& text) = 0;
	virtual bool updateSize () = 0;
	virtual bool drawsPlaceholder () const = 0;

//-----------------------------------------------------------------------------
protected:
	explicit IPlatformTextEdit (IPlatformTextEditCallback* textEdit) : textEdit (textEdit) {}
	IPlatformTextEditCallback* textEdit;
};

} // VSTGUI

/// @endcond
