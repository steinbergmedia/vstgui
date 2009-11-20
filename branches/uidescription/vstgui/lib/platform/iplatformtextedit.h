
#ifndef __iplatformtextedit__
#define __iplatformtextedit__

#include "../cfont.h"
#include "../ccolor.h"
#include "../crect.h"
#include "../cdrawcontext.h"

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
	virtual const char* platformGetText () const = 0;
	virtual CRect platformGetSize () const = 0;
	virtual CRect platformGetVisibleSize () const = 0;
	virtual CPoint platformGetTextInset () const = 0;
	virtual void platformLooseFocus (bool returnPressed) = 0;
	virtual bool platformOnKeyDown (const VstKeyCode& key) = 0;
};

//-----------------------------------------------------------------------------
class IPlatformTextEdit : public CBaseObject
{
public:
	virtual bool getText (char* text, long maxSize) = 0;
	virtual bool setText (const char* text) = 0;
	virtual bool updateSize () = 0;

//-----------------------------------------------------------------------------
protected:
	IPlatformTextEdit (IPlatformTextEditCallback* textEdit) : textEdit (textEdit) {}
	IPlatformTextEditCallback* textEdit;
};

} // namespace

#endif // __iplatformtextedit__
