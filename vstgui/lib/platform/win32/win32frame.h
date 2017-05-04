// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __win32frame__
#define __win32frame__

#include "../../cframe.h"

#if WINDOWS

#include "../iplatformframe.h"
#include "../iplatformviewlayer.h"
#include <windows.h>

namespace VSTGUI {

//-----------------------------------------------------------------------------
class Win32Frame : public IPlatformFrame
{
public:
	Win32Frame (IPlatformFrameCallback* frame, const CRect& size, HWND parent, PlatformType parentType);
	~Win32Frame () noexcept;

	HWND getPlatformWindow () const { return windowHandle; }
	HWND getParentPlatformWindow () const { return parentWindow; }
	HWND getOuterWindow () const;
	IPlatformFrameCallback* getFrame () const { return frame; }
	
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
	DragResult doDrag (IDataPackage* source, const CPoint& offset, CBitmap* dragBitmap) override;

	void setClipboard (const SharedPointer<IDataPackage>& data) override;
	SharedPointer<IDataPackage> getClipboard () override;
	PlatformType getPlatformType () const override { return PlatformType::kHWND; }

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

	COffscreenContext* backBuffer;
	CDrawContext* deviceContext;

	CRect paintRect;
	bool inPaint;
	bool mouseInside;

	RGNDATA* updateRegionList;
	DWORD updateRegionListSize;
};

} // namespace

#endif // WINDOWS

#endif // __win32frame__
