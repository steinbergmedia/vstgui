// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE
#pragma once

#include "../../crect.h"
#include "../iplatformframe.h"
#include <memory>
#include <functional>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace X11 {

//------------------------------------------------------------------------
class IEventHandler
{
public:
	virtual void onEvent () = 0;
};

//------------------------------------------------------------------------
class ITimerHandler
{
public:
	virtual void onTimer () = 0;
};

//------------------------------------------------------------------------
class IRunLoop : public AtomicReferenceCounted
{
public:
	virtual bool registerEventHandler (int fd, IEventHandler* handler) = 0;
	virtual bool unregisterEventHandler (IEventHandler* handler) = 0;

	virtual bool registerTimer (uint64_t interval, ITimerHandler* handler) = 0;
	virtual bool unregisterTimer (ITimerHandler* handler) = 0;
};

//------------------------------------------------------------------------
class FrameConfig : public IPlatformFrameConfig
{
public:
	SharedPointer<IRunLoop> runLoop;
};

//------------------------------------------------------------------------
class Frame : public IPlatformFrame, public IEventHandler
{
public:
	Frame (IPlatformFrameCallback* frame, const CRect& size, uint32_t parent,
		   IPlatformFrameConfig* config);
	~Frame ();

	bool getGlobalPosition (CPoint& pos) const override;
	bool setSize (const CRect& newSize) override;
	bool getSize (CRect& size) const override;
	bool getCurrentMousePosition (CPoint& mousePosition) const override;
	bool getCurrentMouseButtons (CButtonState& buttons) const override;
	bool setMouseCursor (CCursorType type) override;
	bool invalidRect (const CRect& rect) override;
	bool scrollRect (const CRect& src, const CPoint& distance) override;
	bool showTooltip (const CRect& rect, const char* utf8Text) override;
	bool hideTooltip () override;
	void* getPlatformRepresentation () const override;
	SharedPointer<IPlatformTextEdit> createPlatformTextEdit (
		IPlatformTextEditCallback* textEdit) override;
	SharedPointer<IPlatformOptionMenu> createPlatformOptionMenu () override;
#if VSTGUI_OPENGL_SUPPORT
	SharedPointer<IPlatformOpenGLView> createPlatformOpenGLView () override;
#endif
	SharedPointer<IPlatformViewLayer> createPlatformViewLayer (
		IPlatformViewLayerDelegate* drawDelegate, IPlatformViewLayer* parentLayer) override;
	SharedPointer<COffscreenContext> createOffscreenContext (CCoord width, CCoord height,
															 double scaleFactor) override;
	DragResult doDrag (IDataPackage* source, const CPoint& offset, CBitmap* dragBitmap) override;
	void setClipboard (const SharedPointer<IDataPackage>& data) override;
	SharedPointer<IDataPackage> getClipboard () override;

	PlatformType getPlatformType () const override;

	void onEvent () override;

	void* getGtkWindow (); // return is Gtk::Window*
private:
	struct Impl;
	std::unique_ptr<Impl> impl;
};

//------------------------------------------------------------------------
} // X11
} // VSTGUI
