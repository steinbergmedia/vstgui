// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

/// @cond ignore

#include "../vstguifwd.h"
#include "../dragging.h"
#include "../optional.h"
#include "../cstring.h"
#include "iplatformframecallback.h"

namespace VSTGUI {

struct GenericOptionMenuTheme;

//-----------------------------------------------------------------------------
class IPlatformFrame : public AtomicReferenceCounted
{
public:
	/** get the top left position in global coordinates */
	virtual bool getGlobalPosition (CPoint& pos) const = 0;
	/** set size of platform representation relative to parent */
	virtual bool setSize (const CRect& newSize) = 0;
	/** get size of platform representation relative to parent */
	virtual bool getSize (CRect& size) const = 0;

	/** get current mouse position out of event stream */
	virtual bool getCurrentMousePosition (CPoint& mousePosition) const = 0;
	/** get current mouse buttons out of event stream */
	virtual bool getCurrentMouseButtons (CButtonState& buttons) const = 0;
	/** set mouse cursor shape */
	virtual bool setMouseCursor (CCursorType type) = 0;

	/** invalidates rect in platform representation*/
	virtual bool invalidRect (const CRect& rect) = 0;
	/** blit scroll the src rect by distance, return false if not supported */
	virtual bool scrollRect (const CRect& src, const CPoint& distance) = 0;

	/** show tooltip */
	virtual bool showTooltip (const CRect& rect, const char* utf8Text) = 0;
	/** hide tooltip */
	virtual bool hideTooltip () = 0;

	/** TODO: remove this call later when everything is done */
	virtual void* getPlatformRepresentation () const = 0;

	/** create a native text edit control */
	virtual SharedPointer<IPlatformTextEdit>
	createPlatformTextEdit (IPlatformTextEditCallback* textEdit) = 0;
	/** create a native popup menu */
	virtual SharedPointer<IPlatformOptionMenu> createPlatformOptionMenu () = 0;
#if VSTGUI_OPENGL_SUPPORT
	/** create a native opengl sub view */
	virtual SharedPointer<IPlatformOpenGLView> createPlatformOpenGLView () = 0;
#endif // VSTGUI_OPENGL_SUPPORT

	/** create a native view layer, may return 0 if not supported */
	virtual SharedPointer<IPlatformViewLayer> createPlatformViewLayer (
		IPlatformViewLayerDelegate* drawDelegate, IPlatformViewLayer* parentLayer = nullptr) = 0;

#if VSTGUI_ENABLE_DEPRECATED_METHODS
	/** start a drag operation */
	virtual DragResult doDrag (IDataPackage* source, const CPoint& offset, CBitmap* dragBitmap) = 0;
#endif
	/** start a drag operation
	 *
	 *	optional callback will be remembered until the drag is droped or canceled
	 */
	virtual bool doDrag (const DragDescription& dragDescription,
						 const SharedPointer<IDragCallback>& callback) = 0;

	/** */
	virtual PlatformType getPlatformType () const = 0;

	/** called from IPlatformFrameCallback when it's closed */
	virtual void onFrameClosed () = 0;

	/** when called from a key down/up event converts the event to the actual text. */
	virtual Optional<UTF8String> convertCurrentKeyEventToText () = 0;

	/** setup to use (or not) the generic option menu and optionally set the theme to use */
	virtual bool setupGenericOptionMenu (bool use, GenericOptionMenuTheme* theme = nullptr) = 0;

	//-----------------------------------------------------------------------------
protected:
	explicit IPlatformFrame (IPlatformFrameCallback* frame) : frame (frame) {}
	IPlatformFrameCallback* frame;
};

//-----------------------------------------------------------------------------
/* Extension to support Mac TouchBar */
//-----------------------------------------------------------------------------
class ITouchBarCreator : public AtomicReferenceCounted
{
public:
	/** must return an instance of NSTouchBar or nullptr. */
	virtual void* createTouchBar () = 0;
};

//-----------------------------------------------------------------------------
class IPlatformFrameTouchBarExtension /* Extents IPlatformFrame */
{
public:
	virtual ~IPlatformFrameTouchBarExtension () noexcept = default;

	/** set the touchbar creator. */
	virtual void setTouchBarCreator (const SharedPointer<ITouchBarCreator>& creator) = 0;
	/** forces the touchbar to be recreated. */
	virtual void recreateTouchBar () = 0;
};

} // VSTGUI

/// @endcond
