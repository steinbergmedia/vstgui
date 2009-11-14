
#ifndef __nsviewframe__
#define __nsviewframe__

#include "../../../cframe.h"

#if MAC_COCOA && VSTGUI_PLATFORM_ABSTRACTION

#ifdef __OBJC__
@class NSView;
#else
struct NSView;
#endif

BEGIN_NAMESPACE_VSTGUI
class CocoaTooltipWindow;

//-----------------------------------------------------------------------------
class NSViewFrame : public IPlatformFrame
{
public:
	NSViewFrame (IPlatformFrameCallback* frame, const CRect& size, void* parent);
	~NSViewFrame ();

	NSView* getPlatformControl () const { return nsView; }

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
	void* getPlatformRepresentation () const { return nsView; }
	IPlatformTextEdit* createPlatformTextEdit (IPlatformTextEditCallback* textEdit);
	IPlatformOptionMenu* createPlatformOptionMenu ();
	COffscreenContext* createOffscreenContext (CCoord width, CCoord height);
//-----------------------------------------------------------------------------
protected:
	static void initClass ();

	NSView* nsView;
	CocoaTooltipWindow* tooltipWindow;
};

END_NAMESPACE_VSTGUI

#endif // MAC_COCOA && VSTGUI_PLATFORM_ABSTRACTION
#endif // __nsviewframe__
