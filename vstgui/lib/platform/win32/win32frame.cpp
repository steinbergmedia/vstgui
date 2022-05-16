// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "win32frame.h"

#if WINDOWS

#include <commctrl.h>
#include <cmath>
#include <windowsx.h>
#include "direct2d/d2ddrawcontext.h"
#include "direct2d/d2dbitmap.h"
#include "direct2d/d2dgraphicspath.h"
#include "win32factory.h"
#include "win32textedit.h"
#include "win32optionmenu.h"
#include "win32support.h"
#include "win32datapackage.h"
#include "win32dragging.h"
#include "win32directcomposition.h"
#include "win32viewlayer.h"
#include "../common/genericoptionmenu.h"
#include "../common/generictextedit.h"
#include "../../cdropsource.h"
#include "../../cgradient.h"
#include "../../cinvalidrectlist.h"
#include "../../events.h"
#include "../../finally.h"

#include <d2d1_1.h>

#if VSTGUI_OPENGL_SUPPORT
#include "win32openglview.h"
#endif

// windows libraries VSTGUI depends on
#ifdef _MSC_VER
#pragma comment(lib, "Shlwapi.lib")
#endif

namespace VSTGUI {

#define DEBUG_DRAWING	0

//-----------------------------------------------------------------------------
static TCHAR gClassName[100];
static bool bSwapped_mouse_buttons = false; 

//-----------------------------------------------------------------------------
static bool isParentLayered (HWND parent)
{
	WINDOWINFO info;
	info.cbSize = sizeof (info);
	while (parent)
	{
		if (GetWindowInfo (parent, &info))
		{
			if (info.dwStyle & WS_CHILD)
				parent = GetParent (parent);
			else
				break;
		}
	}
	if (parent)
	{
		if (info.dwExStyle & WS_EX_LAYERED)
			return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
Win32Frame::Win32Frame (IPlatformFrameCallback* frame, const CRect& size, HWND parent, PlatformType parentType)
: IPlatformFrame (frame)
, parentWindow (parent)
, windowHandle (nullptr)
, tooltipWindow (nullptr)
, oldFocusWindow (nullptr)
, deviceContext (nullptr)
, inPaint (false)
, mouseInside (false)
, updateRegionList (nullptr)
, updateRegionListSize (0)
{
	useD2D ();
	auto dcFactory = getPlatformFactory ().asWin32Factory ()->getDirectCompositionFactory ();
	if (parentType == PlatformType::kHWNDTopLevel)
	{
		windowHandle = parent;
		parentWindow = nullptr;
		RegisterDragDrop (windowHandle, new CDropTarget (this));
	}
	else
	{
		initWindowClass ();

		DWORD exStyle = isParentLayered (parent) ? WS_EX_TRANSPARENT : 0;
		DWORD style = WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
		if (dcFactory)
			exStyle = WS_EX_NOREDIRECTIONBITMAP;

		windowHandle = CreateWindowEx (exStyle, gClassName, TEXT ("Window"), style, 0, 0,
									   (int)size.getWidth (), (int)size.getHeight (), parentWindow,
									   nullptr, GetInstance (), nullptr);

		if (windowHandle)
		{
			SetWindowLongPtr (windowHandle, GWLP_USERDATA, (__int3264)(LONG_PTR)this);
			RegisterDragDrop (windowHandle, new CDropTarget (this));
		}
	}
	setMouseCursor (kCursorDefault);
	if (dcFactory)
	{
		directCompositionVisual = dcFactory->createVisualForHWND (windowHandle);
	}
}

//-----------------------------------------------------------------------------
Win32Frame::~Win32Frame () noexcept
{
	if (directCompositionVisual)
	{
		if (auto dcFactory = getPlatformFactory ().asWin32Factory ()->getDirectCompositionFactory ())
			dcFactory->removeVisual (directCompositionVisual);
		directCompositionVisual = nullptr;
	}

	if (updateRegionList)
		std::free (updateRegionList);
	if (deviceContext)
		deviceContext->forget ();
	if (tooltipWindow)
		DestroyWindow (tooltipWindow);
	if (backBuffer)
		backBuffer = nullptr;
	if (windowHandle)
		RevokeDragDrop (windowHandle);
	if (parentWindow)
	{
		if (windowHandle)
		{
			SetWindowLongPtr (windowHandle, GWLP_USERDATA, (LONG_PTR)NULL);
			DestroyWindow (windowHandle);
		}
		destroyWindowClass ();
	}

	unuseD2D ();
}

//-----------------------------------------------------------------------------
int32_t Win32Frame::gUseCount = 0;

//-----------------------------------------------------------------------------
void Win32Frame::initWindowClass ()
{
	gUseCount++;
	if (gUseCount == 1)
	{
		OleInitialize (nullptr);

		VSTGUI_SPRINTF (gClassName, TEXT("VSTGUI%p"), GetInstance ());
		
		WNDCLASS windowClass;
		windowClass.style = CS_GLOBALCLASS | CS_DBLCLKS;//|CS_OWNDC; // add Private-DC constant 

		windowClass.lpfnWndProc = WindowProc; 
		windowClass.cbClsExtra  = 0; 
		windowClass.cbWndExtra  = 0; 
		windowClass.hInstance   = GetInstance ();
		windowClass.hIcon = nullptr; 

		windowClass.hCursor = LoadCursor (nullptr, IDC_ARROW);
		#if DEBUG_DRAWING
		windowClass.hbrBackground = GetSysColorBrush (COLOR_BTNFACE);
		#else
		windowClass.hbrBackground = nullptr;
		#endif
		windowClass.lpszMenuName  = nullptr; 
		windowClass.lpszClassName = gClassName; 
		RegisterClass (&windowClass);

		bSwapped_mouse_buttons = GetSystemMetrics (SM_SWAPBUTTON) > 0;
	}
}

//-----------------------------------------------------------------------------
void Win32Frame::destroyWindowClass ()
{
	gUseCount--;
	if (gUseCount == 0)
	{
		UnregisterClass (gClassName, GetInstance ());
		OleUninitialize ();
	}
}

//-----------------------------------------------------------------------------
void Win32Frame::initTooltip ()
{
	if (tooltipWindow == nullptr && windowHandle)
	{
		TOOLINFO    ti;
		// Create the ToolTip control.
		HWND hwndTT = CreateWindow (TOOLTIPS_CLASS, TEXT(""),
							  WS_POPUP,
							  CW_USEDEFAULT, CW_USEDEFAULT,
							  CW_USEDEFAULT, CW_USEDEFAULT,
							  nullptr, (HMENU)nullptr, GetInstance (),
							  nullptr);

		// Prepare TOOLINFO structure for use as tracking ToolTip.
		ti.cbSize = sizeof(TOOLINFO);
		ti.uFlags = TTF_SUBCLASS;
		ti.hwnd   = (HWND)windowHandle;
		ti.uId    = (UINT)0;
		ti.hinst  = GetInstance ();
		ti.lpszText  = const_cast<TCHAR*> (TEXT("This is a tooltip"));
		ti.rect.left = ti.rect.top = ti.rect.bottom = ti.rect.right = 0;

		// Add the tool to the control
		if (!SendMessage (hwndTT, TTM_ADDTOOL, 0, (LPARAM)&ti))
		{
			DestroyWindow (hwndTT);
			return;
		}

		tooltipWindow = hwndTT;
	}
}

//-----------------------------------------------------------------------------
HWND Win32Frame::getOuterWindow () const
{
	int diffWidth, diffHeight;
	RECT  rctTempWnd, rctPluginWnd;
	HWND  hTempWnd = windowHandle;
	GetWindowRect (hTempWnd, &rctPluginWnd);
    
	while (hTempWnd != nullptr)
	{
		// Looking for caption bar
		if (GetWindowLong (hTempWnd, GWL_STYLE) & WS_CAPTION)
			return hTempWnd;

		// Looking for last parent
		if (!GetParent (hTempWnd))
			return hTempWnd;
    
		// get difference between plugin-window and current parent
		GetWindowRect (GetParent (hTempWnd), &rctTempWnd);
	    
		diffWidth  = (rctTempWnd.right - rctTempWnd.left) - (rctPluginWnd.right - rctPluginWnd.left);
		diffHeight = (rctTempWnd.bottom - rctTempWnd.top) - (rctPluginWnd.bottom - rctPluginWnd.top);
		
		// Looking for size mismatch
		if ((abs (diffWidth) > 60) || (abs (diffHeight) > 60)) // parent belongs to host
			return (hTempWnd);

		if (diffWidth < 0)
			diffWidth = 0;
        if (diffHeight < 0)
			diffHeight = 0; 
		
		// get the next parent window
		hTempWnd = GetParent (hTempWnd);
	}

	return nullptr;
}

// IPlatformFrame
//-----------------------------------------------------------------------------
bool Win32Frame::getGlobalPosition (CPoint& pos) const
{
	RECT r;
	GetWindowRect (windowHandle, &r);
	pos.x = r.left;
	pos.y = r.top;
	return true;
}

//-----------------------------------------------------------------------------
bool Win32Frame::setSize (const CRect& newSize)
{
	if (deviceContext)
	{
		deviceContext->forget ();
		deviceContext = nullptr;
	}
	if (backBuffer)
	{
		backBuffer = getPlatformFactory ().createOffscreenContext (newSize.getSize ());
	}
	if (!parentWindow)
		return true;
	SetWindowPos (windowHandle, HWND_TOP, (int)newSize.left, (int)newSize.top, (int)newSize.getWidth (), (int)newSize.getHeight (), SWP_NOZORDER|SWP_NOCOPYBITS|SWP_NOREDRAW|SWP_DEFERERASE);
	invalidRect (newSize);
	return true;
}

//-----------------------------------------------------------------------------
bool Win32Frame::getSize (CRect& size) const
{
	RECT r;
	GetWindowRect (windowHandle, &r);
	POINT p;
	p.x = r.left;
	p.y = r.top;
	MapWindowPoints (HWND_DESKTOP, windowHandle, &p, 1);
	MapWindowPoints (windowHandle, GetParent (windowHandle), &p, 1);
	size.left = p.x;
	size.top = p.y;
	size.right = p.x + (r.right - r.left);
	size.bottom = p.y + (r.bottom - r.top);
	return true;
}

//-----------------------------------------------------------------------------
bool Win32Frame::getCurrentMousePosition (CPoint& mousePosition) const
{
	POINT _where;
	GetCursorPos (&_where);
	mousePosition (static_cast<CCoord> (_where.x), static_cast<CCoord> (_where.y));
	if (auto hwnd = getHWND ())
	{
		ScreenToClient (hwnd, &_where);
		mousePosition (static_cast<CCoord> (_where.x), static_cast<CCoord> (_where.y));
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
bool Win32Frame::getCurrentModifiers (Modifiers& modifiers) const
{
	modifiers.clear ();
	if (GetAsyncKeyState (VK_SHIFT) < 0)
		modifiers.add (ModifierKey::Shift);
	if (GetAsyncKeyState (VK_CONTROL) < 0)
		modifiers.add (ModifierKey::Control);
	if (GetAsyncKeyState (VK_MENU) < 0)
		modifiers.add (ModifierKey::Alt);
	if (GetAsyncKeyState (VK_LWIN) < 0)
		modifiers.add (ModifierKey::Super);
	return true;
}

//-----------------------------------------------------------------------------
bool Win32Frame::getCurrentMouseButtons (CButtonState& buttons) const
{
	if (GetAsyncKeyState (VK_LBUTTON) < 0)
		buttons |= (bSwapped_mouse_buttons ? kRButton : kLButton);
	if (GetAsyncKeyState (VK_MBUTTON) < 0)
		buttons |= kMButton;
	if (GetAsyncKeyState (VK_RBUTTON) < 0)
		buttons |= (bSwapped_mouse_buttons ? kLButton : kRButton);
	if (GetAsyncKeyState (VK_SHIFT)   < 0)
		buttons |= kShift;
	if (GetAsyncKeyState (VK_CONTROL) < 0)
		buttons |= kControl;
	if (GetAsyncKeyState (VK_MENU)    < 0)
		buttons |= kAlt;
	return true;
}

//-----------------------------------------------------------------------------
bool Win32Frame::setMouseCursor (CCursorType type)
{
	HCURSOR cursor = nullptr;
	switch (type)
	{
		case kCursorWait:
			cursor = LoadCursor (nullptr, IDC_WAIT);
			break;
		case kCursorHSize:
			cursor = LoadCursor (nullptr, IDC_SIZEWE);
			break;
		case kCursorVSize:
			cursor = LoadCursor (nullptr, IDC_SIZENS);
			break;
		case kCursorNESWSize:
			cursor = LoadCursor (nullptr, IDC_SIZENESW);
			break;
		case kCursorNWSESize:
			cursor = LoadCursor (nullptr, IDC_SIZENWSE);
			break;
		case kCursorSizeAll:
			cursor = LoadCursor (nullptr, IDC_SIZEALL);
			break;
		case kCursorNotAllowed:
			cursor = LoadCursor (nullptr, IDC_NO);
			break;
		case kCursorHand:
			cursor = LoadCursor (nullptr, IDC_HAND);
			break;
		default:
			cursor = LoadCursor (nullptr, IDC_ARROW);
			break;
	}
	lastSetCursor = type;
	SetClassLongPtr (getPlatformWindow (), GCLP_HCURSOR, (__int3264)(LONG_PTR)(cursor));
	return true;
}

//-----------------------------------------------------------------------------
bool Win32Frame::invalidRect (const CRect& rect)
{
	if (inPaint)
		return false;
	if (!rect.isEmpty ())
	{
		RECT r = {(LONG)rect.left, (LONG)rect.top, (LONG)ceil (rect.right), (LONG)ceil (rect.bottom)};
		InvalidateRect (windowHandle, &r, true);
	}
	return true;
}

//-----------------------------------------------------------------------------
bool Win32Frame::scrollRect (const CRect& src, const CPoint& distance)
{
	return false;
}

//-----------------------------------------------------------------------------
bool Win32Frame::showTooltip (const CRect& rect, const char* utf8Text)
{
	initTooltip ();
	if (tooltipWindow)
	{
		std::string str (utf8Text);
		size_t pos = 0;
		while ((pos = str.find ("\\n", pos)) != std::string::npos)
		{
			str.erase (pos, 2);
			str.insert (pos, "\r\n");
		}
		UTF8StringHelper tooltipText (str.data ());
		RECT rc = RECTfromRect (rect);
		TOOLINFO ti = {};
		ti.cbSize = sizeof(TOOLINFO);
		ti.hwnd = windowHandle;
		ti.uId = 0;
		ti.rect = rc;
		ti.lpszText = (TCHAR*)(const TCHAR*)tooltipText;
		SendMessage (tooltipWindow, TTM_SETMAXTIPWIDTH, 0, 0);
		SendMessage (tooltipWindow, TTM_SETDELAYTIME, 0, 2000);
		SendMessage (tooltipWindow, TTM_UPDATETIPTEXT, 0, (LPARAM)&ti);
		SendMessage (tooltipWindow, TTM_NEWTOOLRECT, 0, (LPARAM)&ti);
		SendMessage (tooltipWindow, TTM_POPUP, 0, 0);
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
bool Win32Frame::hideTooltip ()
{
	if (tooltipWindow)
	{
		TOOLINFO ti = {};
		ti.cbSize = sizeof(TOOLINFO);
		ti.hwnd = windowHandle;
		ti.uId = 0;
		ti.lpszText = nullptr;
		SendMessage (tooltipWindow, TTM_UPDATETIPTEXT, 0, (LPARAM)&ti);
		SendMessage (tooltipWindow, TTM_POP, 0, 0);
	}
	return false;
}

//-----------------------------------------------------------------------------
SharedPointer<IPlatformTextEdit> Win32Frame::createPlatformTextEdit (IPlatformTextEditCallback* textEdit)
{
	if (auto win32Factory = getPlatformFactory ().asWin32Factory ())
	{
		if (win32Factory->useGenericTextEdit ())
			return makeOwned<GenericTextEdit> (textEdit);
	}
	return owned<IPlatformTextEdit> (new Win32TextEdit (windowHandle, textEdit));
}

//-----------------------------------------------------------------------------
SharedPointer<IPlatformOptionMenu> Win32Frame::createPlatformOptionMenu ()
{
	if (genericOptionMenuTheme)
	{
		CButtonState buttons;
		getCurrentMouseButtons (buttons);
		MouseEventButtonState buttonState;
		if (buttons.isLeftButton ())
			buttonState.set (MouseButton::Left);
		else if (buttons.isRightButton ())
			buttonState.set (MouseButton::Right);
		return makeOwned<GenericOptionMenu> (dynamic_cast<CFrame*> (frame), buttonState,
		                                     *genericOptionMenuTheme);
	}
	return owned<IPlatformOptionMenu> (new Win32OptionMenu (windowHandle));
}

//------------------------------------------------------------------------
SharedPointer<IPlatformViewLayer> Win32Frame::createPlatformViewLayer (
	IPlatformViewLayerDelegate* drawDelegate, IPlatformViewLayer* parentLayer)
{
	if (!directCompositionVisual)
		return nullptr; // not supported when not using DirectComposition
	auto parentWin32ViewLayer = dynamic_cast<Win32ViewLayer*> (parentLayer);
	auto parent =
		parentWin32ViewLayer ? parentWin32ViewLayer->getVisual () : directCompositionVisual;
	if (parent)
	{
		auto visual = getPlatformFactory ()
						  .asWin32Factory ()
						  ->getDirectCompositionFactory ()
						  ->createChildVisual (parent, 100, 100);
		auto newLayer =
			makeOwned<Win32ViewLayer> (visual, drawDelegate, [this] (Win32ViewLayer* layer) {
				auto it = std::find (viewLayers.begin (), viewLayers.end (), layer);
				vstgui_assert (it != viewLayers.end ());
				if (it != viewLayers.end ())
					viewLayers.erase (it);
			});
		viewLayers.push_back (newLayer);
		return newLayer;
	}
	return nullptr;
}

#if VSTGUI_OPENGL_SUPPORT
//-----------------------------------------------------------------------------
SharedPointer<IPlatformOpenGLView> Win32Frame::createPlatformOpenGLView ()
{
	return owned<IPlatformOpenGLView> (new Win32OpenGLView (this));
}
#endif

#if VSTGUI_ENABLE_DEPRECATED_METHODS
class Win32LegacyDragSupport final : virtual public DragCallbackAdapter, virtual public NonAtomicReferenceCounted
{
public:
	void dragEnded (IDraggingSession*, CPoint, DragOperation r) final { result = r; }
	DragOperation result {DragOperation::None};
};

//------------------------------------------------------------------------------------
DragResult Win32Frame::doDrag (IDataPackage* source, const CPoint& offset, CBitmap* dragBitmap)
{
	Win32LegacyDragSupport dragSupport;

	Win32DraggingSession session (this);
	if (session.doDrag (DragDescription (source, offset, dragBitmap), &dragSupport))
	{
		switch (dragSupport.result)
		{
			case DragOperation::Copy: return kDragCopied;
			case DragOperation::Move: return kDragMoved;
			case DragOperation::None: return kDragRefused;
		}
	}
	return kDragRefused;
}
#endif

//-----------------------------------------------------------------------------
bool Win32Frame::doDrag (const DragDescription& dragDescription, const SharedPointer<IDragCallback>& callback)
{
	Win32DraggingSession session (this);
	return session.doDrag (dragDescription, callback);
}

//-----------------------------------------------------------------------------
void Win32Frame::onFrameClosed ()
{
	frame = nullptr;
}

//-----------------------------------------------------------------------------
bool Win32Frame::setupGenericOptionMenu (bool use, GenericOptionMenuTheme* theme)
{
	if (!use)
	{
		genericOptionMenuTheme = nullptr;
	}
	else
	{
		if (theme)
			genericOptionMenuTheme = std::make_unique<GenericOptionMenuTheme> (*theme);
		else
			genericOptionMenuTheme = std::make_unique<GenericOptionMenuTheme> ();
	}
	return true;
}

//-----------------------------------------------------------------------------
template<typename Proc>
void Win32Frame::iterateRegion (HRGN rgn, Proc func)
{
	DWORD len = GetRegionData (rgn, 0, nullptr);
	if (len)
	{
		if (len > updateRegionListSize)
		{
			if (updateRegionList)
				std::free (updateRegionList);
			updateRegionListSize = len;
			updateRegionList = (RGNDATA*)std::malloc (updateRegionListSize);
		}
		GetRegionData (rgn, len, updateRegionList);
		if (updateRegionList->rdh.nCount > 0)
		{
			CInvalidRectList dirtyRects;
			auto* rp = reinterpret_cast<RECT*> (updateRegionList->Buffer);
			for (uint32_t i = 0; i < updateRegionList->rdh.nCount; ++i, ++rp)
			{
				CRect ur (rp->left, rp->top, rp->right, rp->bottom);
				dirtyRects.add (ur);
			}
			for (auto& _updateRect : dirtyRects)
			{
				func (_updateRect);
			}
		}
		else
		{
			RECT r;
			GetRgnBox (rgn, &r);
			auto updateRect = rectFromRECT (r);
			func (updateRect);
		}
	}
}

//-----------------------------------------------------------------------------
void Win32Frame::paint (HWND hwnd)
{
	HRGN rgn = CreateRectRgn (0, 0, 0, 0);
	if (GetUpdateRgn (hwnd, rgn, false) == NULLREGION)
	{
		DeleteObject (rgn);
		return;
	}

	inPaint = true;
	bool needsInvalidation = false;
	CRect frameSize;

	PAINTSTRUCT ps;
	if (HDC hdc = BeginPaint (hwnd, &ps))
	{
		RECT clientRect;
		GetClientRect (windowHandle, &clientRect);
		frameSize = rectFromRECT (clientRect);
		if (directCompositionVisual)
		{
			directCompositionVisual->resize (static_cast<uint32_t> (frameSize.getWidth ()),
											 static_cast<uint32_t> (frameSize.getHeight ()));
			iterateRegion (rgn, [&] (const auto& rect) {
				directCompositionVisual->update (
					rect, [&] (auto deviceContext, auto rect, auto offsetX, auto offsetY) {
						COM::Ptr<ID2D1Device> device;
						deviceContext->GetDevice (device.adoptPtr ());
						D2DDrawContext drawContext (deviceContext, frameSize, device.get ());
						drawContext.setClipRect (rect);
						CGraphicsTransform tm;
						tm.translate (offsetX - rect.left, offsetY - rect.top);
						CDrawContext::Transform transform (drawContext, tm);
						{
							drawContext.saveGlobalState ();
							drawContext.clearRect (rect);
							getFrame ()->platformDrawRect (&drawContext, rect);
							drawContext.restoreGlobalState ();
						}
					});
			});
			for (auto& vl : viewLayers)
				vl->drawInvalidRects ();
			if (!directCompositionVisual->commit ())
				needsInvalidation = true;
		}
		else
		{
			if (deviceContext == nullptr)
				deviceContext = createDrawContext (hwnd, hdc, frameSize);
			if (deviceContext)
			{
				GetRgnBox (rgn, &ps.rcPaint);
				CRect updateRect ((CCoord)ps.rcPaint.left, (CCoord)ps.rcPaint.top,
								  (CCoord)ps.rcPaint.right, (CCoord)ps.rcPaint.bottom);
				deviceContext->setClipRect (updateRect);

				CDrawContext* drawContext = backBuffer ? backBuffer : deviceContext;
				drawContext->beginDraw ();

				iterateRegion (rgn, [&] (const auto& rect) {
					drawContext->clearRect (rect);
					getFrame ()->platformDrawRect (drawContext, rect);
				});

				drawContext->endDraw ();
				if (backBuffer)
				{
					deviceContext->beginDraw ();
					deviceContext->clearRect (updateRect);
					backBuffer->copyFrom (deviceContext, updateRect,
										  CPoint (updateRect.left, updateRect.top));
					deviceContext->endDraw ();
				}
			}
		}
	}

	EndPaint (hwnd, &ps);
	DeleteObject (rgn);
	
	inPaint = false;
	if (needsInvalidation && !frameSize.isEmpty ())
	{
		invalidRect (frameSize);
		for (auto& vl : viewLayers)
		{
			vl->invalidRect (vl->getViewSize ());
		}
	}
}

//-----------------------------------------------------------------------------
static void setupMouseEventFromWParam (MouseEvent& event, WPARAM wParam)
{
	if (wParam & MK_LBUTTON)
		event.buttonState.add (MouseButton::Left);
	if (wParam & MK_RBUTTON)
		event.buttonState.add (MouseButton::Right);
	if (wParam & MK_MBUTTON)
		event.buttonState.add (MouseButton::Middle);
	if (wParam & MK_XBUTTON1)
		event.buttonState.add (MouseButton::Fourth);
	if (wParam & MK_XBUTTON2)
		event.buttonState.add (MouseButton::Fifth);
	if (wParam & MK_CONTROL)
		event.modifiers.add (ModifierKey::Control);
	if (wParam & MK_SHIFT)
		event.modifiers.add (ModifierKey::Shift);
	if (GetAsyncKeyState (VK_MENU) < 0)
		event.modifiers.add (ModifierKey::Alt);
	if (GetAsyncKeyState (VK_LWIN) < 0)
		event.modifiers.add (ModifierKey::Super);
}

//-----------------------------------------------------------------------------
LONG_PTR WINAPI Win32Frame::proc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (getFrame () == nullptr)
		return DefWindowProc (hwnd, message, wParam, lParam);

	SharedPointer<Win32Frame> lifeGuard (this);
	IPlatformFrameCallback* pFrame = getFrame ();
	bool doubleClick = false;

	auto oldEvent = std::move (currentEvent);
	auto f = finally ([this, oldEvent = std::move (oldEvent)] () mutable { currentEvent = std::move (oldEvent); });
	currentEvent = Optional<MSG> ({hwnd, message, wParam, lParam});

	switch (message)
	{
		case WM_MOUSEWHEEL:
		case WM_MOUSEHWHEEL:	// new since vista
		{
			MouseWheelEvent wheelEvent;
			updateModifiers (wheelEvent.modifiers);

			short zDelta = (short) GET_WHEEL_DELTA_WPARAM(wParam);
			POINT p {GET_X_LPARAM (lParam), GET_Y_LPARAM (lParam)};
			ScreenToClient (windowHandle, &p);
			wheelEvent.mousePosition = {static_cast<CCoord> (p.x), static_cast<CCoord> (p.y)};
			if (zDelta != WHEEL_DELTA)
				wheelEvent.flags |= MouseWheelEvent::Flags::PreciseDeltas;
			if (message == WM_MOUSEWHEEL)
				wheelEvent.deltaY = static_cast<CCoord> (zDelta) / WHEEL_DELTA;
			else
				wheelEvent.deltaX = static_cast<CCoord> (zDelta) / WHEEL_DELTA;
			pFrame->platformOnEvent (wheelEvent);
			if (wheelEvent.consumed)
				return 0;
			break;
		}
		case WM_CTLCOLOREDIT:
		{
			auto* win32TextEdit = (Win32TextEdit*)(LONG_PTR) GetWindowLongPtr ((HWND)lParam, GWLP_USERDATA);
			if (win32TextEdit)
			{
				CColor fontColor = win32TextEdit->getTextEdit ()->platformGetFontColor ();
				SetTextColor ((HDC) wParam, RGB (fontColor.red, fontColor.green, fontColor.blue));
#if 1 // TODO: I don't know why the transparent part does not work anymore. Needs more investigation.
				CColor backColor = win32TextEdit->getTextEdit ()->platformGetBackColor ();
				SetBkColor ((HDC) wParam, RGB (backColor.red, backColor.green, backColor.blue));
				return (LRESULT)(win32TextEdit->getPlatformBackColor ());
#else
				SetBkMode ((HDC)wParam, TRANSPARENT);
				return (LRESULT) ::GetStockObject (HOLLOW_BRUSH);
#endif
			}
			break;
		}
			
		case WM_PAINT:
		{
			paint (hwnd);
			return 0;
		}
			
		case WM_RBUTTONDBLCLK:
		case WM_MBUTTONDBLCLK:
		case WM_LBUTTONDBLCLK:
		case WM_XBUTTONDBLCLK:
			doubleClick = true;
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_LBUTTONDOWN:
		case WM_XBUTTONDOWN:
		{
			MouseDownEvent event;
			setupMouseEventFromWParam (event, wParam);
			event.clickCount = doubleClick ? 2 : 1;

			HWND oldFocus = SetFocus (getPlatformWindow ());
			if(oldFocus != hwnd)
				oldFocusWindow = oldFocus;

			event.mousePosition (GET_X_LPARAM (lParam), GET_Y_LPARAM (lParam));
			pFrame->platformOnEvent (event);
			if (event.consumed && getPlatformWindow ())
				SetCapture (getPlatformWindow ());
			return 0;
		}
		case WM_MOUSELEAVE:
		{
			MouseExitEvent event;
			getCurrentMousePosition (event.mousePosition);
			getCurrentModifiers (event.modifiers);
			pFrame->platformOnEvent (event);
			mouseInside = false;
			return 0;
		}
		case WM_MOUSEMOVE:
		{
			MouseMoveEvent event;
			setupMouseEventFromWParam (event, wParam);
			if (!mouseInside)
			{
				// this makes sure that WM_MOUSELEAVE will be generated by the system
				mouseInside = true;
				TRACKMOUSEEVENT tme = {};
				tme.cbSize = sizeof (tme);
				tme.dwFlags = TME_LEAVE;
				tme.hwndTrack = windowHandle;
				TrackMouseEvent (&tme);
			}
			event.mousePosition (GET_X_LPARAM (lParam), GET_Y_LPARAM (lParam));
			pFrame->platformOnEvent (event);
			return 0;
		}
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
		case WM_XBUTTONUP:
		{
			MouseUpEvent event;
			setupMouseEventFromWParam (event, wParam);
			
			if (message == WM_LBUTTONUP)
				event.buttonState.add (MouseButton::Left);
			else if (message == WM_RBUTTONUP)
				event.buttonState.add (MouseButton::Right);
			else if (message == WM_MBUTTONUP)
				event.buttonState.add (MouseButton::Middle);

			event.mousePosition (GET_X_LPARAM (lParam), GET_Y_LPARAM (lParam));
			pFrame->platformOnEvent (event);
			ReleaseCapture ();
			return 0;
		}
		case WM_KEYUP: [[fallthrough]];
		case WM_KEYDOWN:
		{
			KeyboardEvent keyEvent;
			if (message == WM_KEYDOWN)
			{
				keyEvent.type = EventType::KeyDown;
				auto repeatCount = (lParam & 0xFFFF);
				if (repeatCount > 1)
					keyEvent.isRepeat = true;
			}
			else
				keyEvent.type = EventType::KeyUp;
			updateModifiers (keyEvent.modifiers);

			keyEvent.virt = translateWinVirtualKey (wParam);
			keyEvent.character = MapVirtualKey (static_cast<UINT> (wParam), MAPVK_VK_TO_CHAR);
			if (keyEvent.virt != VirtualKey::None || keyEvent.character)
			{
				keyEvent.character = std::tolower (keyEvent.character);
				pFrame->platformOnEvent (keyEvent);
				if (keyEvent.consumed)
					return 0;
			}

			if (IsWindow (oldFocusWindow))
			{
				auto oldProc =
				    reinterpret_cast<WNDPROC> (GetWindowLongPtr (oldFocusWindow, GWLP_WNDPROC));
				if (oldProc && oldProc != WindowProc)
					return CallWindowProc (oldProc, oldFocusWindow, message, wParam, lParam);
			}
			break;
		}
		case WM_SETFOCUS:
		{
			pFrame->platformOnActivate (true);
			break;
		}
		case WM_KILLFOCUS:
		{
			oldFocusWindow = nullptr;

			HWND focusWindow = GetFocus ();
			if (GetParent (focusWindow) != windowHandle)
				pFrame->platformOnActivate (false);
			break;
		}
		case WM_DESTROY:
		{
#if DEBUG
			DebugPrint ("This sometimes happens, only when we are currently processing a mouse down event and via a callback into the host the window gets destroyed. Otherwise this should never get called. We are the owner of the window and we are responsible of destroying it.\n");
#endif
			if (parentWindow)
				windowHandle = nullptr;
			break;
		}
		case WM_ERASEBKGND:
		{
			return 1; // don't draw background
		}
		case WM_COMMAND:
		{
			if (HIWORD (wParam) == EN_CHANGE)
			{
				// text control changes will be forwarded to the text control window proc
				HWND controlWindow = (HWND)lParam;
				auto textEditWindowProc = (WINDOWSPROC)(LONG_PTR)GetWindowLongPtr (controlWindow, GWLP_WNDPROC);
				if (textEditWindowProc)
				{
					textEditWindowProc (controlWindow, WM_COMMAND, wParam, lParam);
				}
			}
			break;
		}
	}
	return DefWindowProc (hwnd, message, wParam, lParam);
}

//-----------------------------------------------------------------------------
Optional<UTF8String> Win32Frame::convertCurrentKeyEventToText ()
{
	if (currentEvent && currentEvent->message == WM_KEYDOWN)
	{
		MSG msg;
		if (PeekMessage (&msg, windowHandle, WM_CHAR, WM_CHAR, PM_REMOVE | PM_NOYIELD))
		{
			std::wstring wideStr (1, static_cast<wchar_t> (msg.wParam));
			UTF8StringHelper helper (wideStr.data ());
			return Optional<UTF8String> (UTF8String (helper.getUTF8String ()));
		}
	}
	return {};
}

//-----------------------------------------------------------------------------
LONG_PTR WINAPI Win32Frame::WindowProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	auto* win32Frame = (Win32Frame*)(LONG_PTR)GetWindowLongPtr (hwnd, GWLP_USERDATA);
	if (win32Frame)
	{
		return win32Frame->proc (hwnd, message, wParam, lParam);
	}
	return DefWindowProc (hwnd, message, wParam, lParam);
}

} // VSTGUI

#endif // WINDOWS
