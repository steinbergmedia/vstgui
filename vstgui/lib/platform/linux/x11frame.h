// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE
#pragma once

#include "../../crect.h"
#include "../iplatformframe.h"
#include "../iplatformresourceinputstream.h"
#include "../platform_x11.h"
#include "../common/genericoptionmenu.h"
#include "irunloop.h"
#include <memory>
#include <functional>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace X11 {

//------------------------------------------------------------------------
class Frame
: public IPlatformFrame
, public IX11Frame
, public IGenericOptionMenuListener
{
public:
	Frame (IPlatformFrameCallback* frame, const CRect& size, uint32_t parent,
		   IPlatformFrameConfig* config);
	~Frame ();

private:
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
#endif
	SharedPointer<IPlatformViewLayer> createPlatformViewLayer (
		IPlatformViewLayerDelegate* drawDelegate, IPlatformViewLayer* parentLayer) override;
#if VSTGUI_ENABLE_DEPRECATED_METHODS
	DragResult doDrag (IDataPackage* source, const CPoint& offset, CBitmap* dragBitmap) override;
#endif
	bool doDrag (const DragDescription& dragDescription,
				 const SharedPointer<IDragCallback>& callback) override;

	PlatformType getPlatformType () const override;
	void onFrameClosed () override {}
	Optional<UTF8String> convertCurrentKeyEventToText () override;
	bool setupGenericOptionMenu (bool use, GenericOptionMenuTheme* theme = nullptr) override;

	uint32_t getX11WindowID () const override;

	void optionMenuPopupStarted () override;
	void optionMenuPopupStopped () override;

private:
	struct Impl;
	std::unique_ptr<Impl> impl;
};

//------------------------------------------------------------------------
} // X11
} // VSTGUI
