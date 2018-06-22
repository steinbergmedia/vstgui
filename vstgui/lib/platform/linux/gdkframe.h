// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../../crect.h"
#include "../iplatformframe.h"
#include "../iplatformresourceinputstream.h"
#include "../iplatformtimer.h"
#include "irunloop.h"
#include <memory>
#include <functional>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace GDK {

//------------------------------------------------------------------------
class Frame : public IPlatformFrame
{
public:
	static Frame* create (IPlatformFrameCallback* frame,
						  const CRect& size,
						  void* parent,
						  IPlatformFrameConfig* config);

	using CreateIResourceInputStreamFunc =
		std::function<IPlatformResourceInputStream::Ptr (const CResourceDescription& desc)>;

	static CreateIResourceInputStreamFunc createResourceInputStreamFunc;

	using CreatePlatformTimerFunc =
		std::function<SharedPointer<IPlatformTimer> (IPlatformTimerCallback*)>;

	static CreatePlatformTimerFunc createPlatformTimerFunc;

	void handleEvent (void* gdkEvent);
private:
	Frame (IPlatformFrameCallback* frame);

	~Frame () noexcept override;

	bool init (const CRect& size, void* parent, IPlatformFrameConfig* config);

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
	SharedPointer<IPlatformTextEdit>
	createPlatformTextEdit (IPlatformTextEditCallback* textEdit) override;
	SharedPointer<IPlatformOptionMenu> createPlatformOptionMenu () override;
#if VSTGUI_OPENGL_SUPPORT
	SharedPointer<IPlatformOpenGLView> createPlatformOpenGLView () override;
#endif // VSTGUI_OPENGL_SUPPORT
	SharedPointer<IPlatformViewLayer>
	createPlatformViewLayer (IPlatformViewLayerDelegate* drawDelegate,
							 IPlatformViewLayer* parentLayer = nullptr) override;
	SharedPointer<COffscreenContext> createOffscreenContext (CCoord width,
															 CCoord height,
															 double scaleFactor = 1.) override;
#if VSTGUI_ENABLE_DEPRECATED_METHODS
	DragResult doDrag (IDataPackage* source, const CPoint& offset, CBitmap* dragBitmap) override;
#endif
	bool doDrag (const DragDescription& dragDescription,
				 const SharedPointer<IDragCallback>& callback) override;
	void setClipboard (const SharedPointer<IDataPackage>& data) override;
	SharedPointer<IDataPackage> getClipboard () override;
	PlatformType getPlatformType () const override;
	void onFrameClosed () override;

//--------
	void drawDirtyRegions ();

	struct Impl;
	std::unique_ptr<Impl> impl;
};

} // GDK
} // VSTGUI