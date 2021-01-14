// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../../../vstguifwd.h"
#include "../../../cview.h"
#include "../../iplatformframe.h"

#if TARGET_OS_IPHONE

#ifdef __OBJC__
#import <UIKit/UIKit.h>
#else
struct UIView;
#endif

namespace VSTGUI {

//-----------------------------------------------------------------------------
class UIViewFrame : public IPlatformFrame
{
public:
	UIViewFrame (IPlatformFrameCallback* frame, const CRect& size, UIView* parent);
	~UIViewFrame ();

	UIView* getPlatformControl () const { return uiView; }
	IPlatformFrameCallback* getFrame () const { return frame; }

	// IPlatformFrame
	bool getGlobalPosition (CPoint& pos) const override;
	bool setSize (const CRect& newSize) override;
	bool getSize (CRect& size) const override;
	bool getCurrentMousePosition (CPoint& mousePosition) const override { return false; };
	bool getCurrentMouseButtons (CButtonState& buttons) const override { return false; };
	bool setMouseCursor (CCursorType type) override { return false; };
	bool invalidRect (const CRect& rect) override;
	bool scrollRect (const CRect& src, const CPoint& distance) override;
	bool showTooltip (const CRect& rect, const char* utf8Text) override { return false; };
	bool hideTooltip () override { return false; };
	void* getPlatformRepresentation () const override { return (__bridge void*)uiView; }
	SharedPointer<IPlatformTextEdit> createPlatformTextEdit (IPlatformTextEditCallback* textEdit) override;
	SharedPointer<IPlatformOptionMenu> createPlatformOptionMenu () override;
#if VSTGUI_OPENGL_SUPPORT
	SharedPointer<IPlatformOpenGLView> createPlatformOpenGLView () override;
#endif
	SharedPointer<IPlatformViewLayer> createPlatformViewLayer (IPlatformViewLayerDelegate* drawDelegate, IPlatformViewLayer* parentLayer) override;
#if VSTGUI_ENABLE_DEPRECATED_METHODS
	DragResult doDrag (IDataPackage* source, const CPoint& offset, CBitmap* dragBitmap) override;
#endif
	bool doDrag (const DragDescription& dragDescription, const SharedPointer<IDragCallback>& callback) override;

	PlatformType getPlatformType () const override { return PlatformType::kUIView; }
	void onFrameClosed () override {}
	Optional<UTF8String> convertCurrentKeyEventToText () override { return {}; }
	bool setupGenericOptionMenu (bool use, GenericOptionMenuTheme* theme = nullptr) override { return false; }

//-----------------------------------------------------------------------------
protected:
	UIView* uiView;
};

} // VSTGUI

#endif // TARGET_OS_IPHONE
