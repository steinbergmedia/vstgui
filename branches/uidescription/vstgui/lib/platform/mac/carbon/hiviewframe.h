
#ifndef __hiviewframe__
#define __hiviewframe__

#include "../../../cframe.h"

#if MAC_CARBON

#include <Carbon/Carbon.h>

namespace VSTGUI {

//-----------------------------------------------------------------------------
class HIViewFrame : public IPlatformFrame
{
public:
	HIViewFrame (IPlatformFrameCallback* frame, const CRect& size, WindowRef parent);
	~HIViewFrame ();

	HIViewRef getPlatformControl () const { return controlRef; }
	const CPoint& getScrollOffset () const { return hiScrollOffset; }

	// IPlatformFrame
	bool getGlobalPosition (CPoint& pos) const;
	bool setSize (const CRect& newSize);
	bool getSize (CRect& size) const;
	bool getCurrentMousePosition (CPoint& mousePosition) const;
	bool getCurrentMouseButtons (long& buttons) const;
	bool setMouseCursor (CCursorType type);
	bool invalidRect (const CRect& rect);
	bool scrollRect (const CRect& src, const CPoint& distance);
	unsigned long getTicks () const;
	bool showTooltip (const CRect& rect, const char* utf8Text);
	bool hideTooltip ();
	void* getPlatformRepresentation () const { return controlRef; }
	IPlatformTextEdit* createPlatformTextEdit (IPlatformTextEditCallback* textEdit);
	IPlatformOptionMenu* createPlatformOptionMenu ();
	COffscreenContext* createOffscreenContext (CCoord width, CCoord height);
	CGraphicsPath* createGraphicsPath ();

//-----------------------------------------------------------------------------
protected:
	static pascal OSStatus carbonMouseEventHandler (EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData);
	static pascal OSStatus carbonEventHandler (EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData);
	
	WindowRef window;
	HIViewRef controlRef;
	bool hasFocus;
	bool isInMouseTracking;
	EventHandlerRef mouseEventHandler;
	CPoint hiScrollOffset;
};

} // namespace

#endif // MAC_CARBON
#endif // __hiviewframe__
