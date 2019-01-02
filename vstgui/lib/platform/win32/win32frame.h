// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../platform_win32.h"

#if WINDOWS

#include "../../cframe.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
class Win32Frame : public IPlatformFrame, public IWin32PlatformFrame
{
public:
	Win32Frame (IPlatformFrameCallback* frame, const CRect& size, HWND parent, PlatformType parentType);
	~Win32Frame () noexcept;

	HWND getHWND () const override { return windowHandle; }
	HWND getPlatformWindow () const { return windowHandle; }
	HWND getParentPlatformWindow () const { return parentWindow; }
	HWND getOuterWindow () const;
	IPlatformFrameCallback* getFrame () const { return frame; }
	
	CCursorType getLastSetCursor () const { return lastSetCursor; }

	// IPlatformFrame
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
	void* getPlatformRepresentation () const override { return windowHandle; }
	SharedPointer<IPlatformTextEdit> createPlatformTextEdit (IPlatformTextEditCallback* textEdit) override;
	SharedPointer<IPlatformOptionMenu> createPlatformOptionMenu () override;
#if VSTGUI_OPENGL_SUPPORT
	SharedPointer<IPlatformOpenGLView> createPlatformOpenGLView () override;
#endif
	SharedPointer<IPlatformViewLayer> createPlatformViewLayer (IPlatformViewLayerDelegate* drawDelegate, IPlatformViewLayer* parentLayer = nullptr) override { return 0; } // not yet supported
	SharedPointer<COffscreenContext> createOffscreenContext (CCoord width, CCoord height, double scaleFactor = 1.) override;
#if VSTGUI_ENABLE_DEPRECATED_METHODS
	DragResult doDrag (IDataPackage* source, const CPoint& offset, CBitmap* dragBitmap) override;
#endif
	bool doDrag (const DragDescription& dragDescription, const SharedPointer<IDragCallback>& callback) override;

	void setClipboard (const SharedPointer<IDataPackage>& data) override;
	SharedPointer<IDataPackage> getClipboard () override;
	PlatformType getPlatformType () const override { return PlatformType::kHWND; }
	void onFrameClosed () override;
	Optional<UTF8String> convertCurrentKeyEventToText () override { return {}; }

	LONG_PTR WINAPI proc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
//-----------------------------------------------------------------------------
protected:
	void initTooltip ();
	void paint (HWND hwnd);

	static void initWindowClass ();
	static void destroyWindowClass ();
	static LONG_PTR WINAPI WindowProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
	static int32_t gUseCount;

	HWND parentWindow;
	HWND windowHandle;
	HWND tooltipWindow;
	HWND oldFocusWindow;

	COffscreenContext* backBuffer;
	CDrawContext* deviceContext;

	bool inPaint;
	bool mouseInside;

	RGNDATA* updateRegionList;
	DWORD updateRegionListSize;
	CCursorType lastSetCursor;
};

} // VSTGUI

#endif // WINDOWS
