
#ifndef __iplatformframe__
#define __iplatformframe__

#include "../cframe.h"

#if VSTGUI_PLATFORM_ABSTRACTION

namespace VSTGUI {
class IPlatformTextEdit;
class IPlatformTextEditCallback;
class IPlatformOptionMenu;

//-----------------------------------------------------------------------------
// Callback interface from IPlatformFrame implementations
//-----------------------------------------------------------------------------
class IPlatformFrameCallback
{
public:
	virtual bool platformDrawRect (CDrawContext* context, const CRect& rect) = 0;
	
	virtual CMouseEventResult platformOnMouseDown (CPoint& where, const long& buttons) = 0;
	virtual CMouseEventResult platformOnMouseMoved (CPoint& where, const long& buttons) = 0;
	virtual CMouseEventResult platformOnMouseUp (CPoint& where, const long& buttons) = 0;
	virtual CMouseEventResult platformOnMouseExited (CPoint& where, const long& buttons) = 0;
	virtual bool platformOnMouseWheel (const CPoint &where, const CMouseWheelAxis &axis, const float &distance, const long &buttons) = 0;

	virtual bool platformOnDrop (CDragContainer* drag, const CPoint& where) = 0;
	virtual void platformOnDragEnter (CDragContainer* drag, const CPoint& where) = 0;
	virtual void platformOnDragLeave (CDragContainer* drag, const CPoint& where) = 0;
	virtual void platformOnDragMove (CDragContainer* drag, const CPoint& where) = 0;

	virtual bool platformOnKeyDown (VstKeyCode& keyCode) = 0;
	virtual bool platformOnKeyUp (VstKeyCode& keyCode) = 0;

	virtual void platformOnActivate (bool state) = 0;
};

//-----------------------------------------------------------------------------
class IPlatformFrame : public CBaseObject
{
public:
	static IPlatformFrame* createPlatformFrame (IPlatformFrameCallback* frame, const CRect& size, void* parent);	///< create platform representation
	
	virtual bool getGlobalPosition (CPoint& pos) const = 0;	///< get the top left position in global coordinates
	virtual bool setSize (const CRect& newSize) = 0;	///< set size of platform representation relative to parent
	virtual bool getSize (CRect& size) const = 0;	///< get size of platform representation relative to parent
	
	virtual bool getCurrentMousePosition (CPoint& mousePosition) const = 0;	///< get current mouse position out of event stream
	virtual bool getCurrentMouseButtons (long& buttons) const = 0;	///< get current mouse buttons out of event stream
	virtual bool setMouseCursor (CCursorType type) = 0;	///< set mouse cursor shape
	
	virtual bool invalidRect (const CRect& rect) = 0;	///< invalidates rect in platform representation
	virtual bool scrollRect (const CRect& src, const CPoint& distance) = 0; ///< blit scroll the src rect by distance, return false if not supported

	virtual unsigned long getTicks () const = 0;

	virtual bool showTooltip (const CRect& rect, const char* utf8Text) = 0; ///< show tooltip
	virtual bool hideTooltip () = 0;	///< hide tooltip

	virtual void* getPlatformRepresentation () const = 0;	// TODO: remove this call later when everything is done

	virtual IPlatformTextEdit* createPlatformTextEdit (IPlatformTextEditCallback* textEdit) = 0; ///< create a native text edit control
	virtual IPlatformOptionMenu* createPlatformOptionMenu () = 0; ///< create a native popup menu
	
	virtual COffscreenContext* createOffscreenContext (CCoord width, CCoord height) = 0; ///< create an offscreen draw device
//-----------------------------------------------------------------------------
protected:
	IPlatformFrame (IPlatformFrameCallback* frame = 0) : frame (frame) {}
	IPlatformFrameCallback* frame;
};

} // namespace

#endif // VSTGUI_PLATFORM_ABSTRACTION
#endif // __iplatformframe__
