// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __iplatformframe__
#define __iplatformframe__

/// @cond ignore

#include "../vstguifwd.h"
#include "iplatformframecallback.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
class IPlatformFrame : public AtomicReferenceCounted
{
public:
	static IPlatformFrame* createPlatformFrame (IPlatformFrameCallback* frame, const CRect& size, void* parent, PlatformType parentType, IPlatformFrameConfig* config = nullptr);	///< create platform representation
	static uint32_t getTicks ();

	virtual bool getGlobalPosition (CPoint& pos) const = 0;	///< get the top left position in global coordinates
	virtual bool setSize (const CRect& newSize) = 0;	///< set size of platform representation relative to parent
	virtual bool getSize (CRect& size) const = 0;	///< get size of platform representation relative to parent
	
	virtual bool getCurrentMousePosition (CPoint& mousePosition) const = 0;	///< get current mouse position out of event stream
	virtual bool getCurrentMouseButtons (CButtonState& buttons) const = 0;	///< get current mouse buttons out of event stream
	virtual bool setMouseCursor (CCursorType type) = 0;	///< set mouse cursor shape
	
	virtual bool invalidRect (const CRect& rect) = 0;	///< invalidates rect in platform representation
	virtual bool scrollRect (const CRect& src, const CPoint& distance) = 0; ///< blit scroll the src rect by distance, return false if not supported

	virtual bool showTooltip (const CRect& rect, const char* utf8Text) = 0; ///< show tooltip
	virtual bool hideTooltip () = 0;	///< hide tooltip

	virtual void* getPlatformRepresentation () const = 0;	// TODO: remove this call later when everything is done

	virtual SharedPointer<IPlatformTextEdit> createPlatformTextEdit (IPlatformTextEditCallback* textEdit) = 0; ///< create a native text edit control
	virtual SharedPointer<IPlatformOptionMenu> createPlatformOptionMenu () = 0; ///< create a native popup menu
#if VSTGUI_OPENGL_SUPPORT
	virtual SharedPointer<IPlatformOpenGLView> createPlatformOpenGLView () = 0; ///< create a native opengl sub view
#endif // VSTGUI_OPENGL_SUPPORT
	
	virtual SharedPointer<IPlatformViewLayer> createPlatformViewLayer (IPlatformViewLayerDelegate* drawDelegate, IPlatformViewLayer* parentLayer = nullptr) = 0; ///< create a native view layer, may return 0 if not supported
	
	virtual SharedPointer<COffscreenContext> createOffscreenContext (CCoord width, CCoord height, double scaleFactor = 1.) = 0; ///< create an offscreen draw device

	virtual DragResult doDrag (IDataPackage* source, const CPoint& offset, CBitmap* dragBitmap) = 0; ///< start a drag operation

	virtual void setClipboard (const SharedPointer<IDataPackage>& data) = 0;	///< set clipboard data
	virtual SharedPointer<IDataPackage> getClipboard () = 0;			///< get clipboard data

	virtual PlatformType getPlatformType () const = 0;
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

} // namespace

/// @endcond

#endif // __iplatformframe__
