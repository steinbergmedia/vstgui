//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 4.3
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2015, Steinberg Media Technologies, All Rights Reserved
//-----------------------------------------------------------------------------
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
// 
//   * Redistributions of source code must retain the above copyright notice, 
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation 
//     and/or other materials provided with the distribution.
//   * Neither the name of the Steinberg Media Technologies nor the names of its
//     contributors may be used to endorse or promote products derived from this 
//     software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#include "hiviewframe.h"

#if MAC_CARBON

#include "../../iplatformtextedit.h"
#include "../../../cbitmap.h"
#include "../../../cdrawcontext.h"
#include "../../../cdropsource.h"
#include "hiviewtextedit.h"
#include "hiviewoptionmenu.h"
#include "../cgdrawcontext.h"
#include "../cgbitmap.h"
#include "../macglobals.h"
#include "../quartzgraphicspath.h"
#include "../macclipboard.h"

#pragma GCC diagnostic ignored "-Wdeprecated-declarations" // we know that we use deprecated functions from Carbon, so we don't want to be warned

namespace VSTGUI {

#if !MAC_COCOA
//-----------------------------------------------------------------------------
IPlatformFrame* IPlatformFrame::createPlatformFrame (IPlatformFrameCallback* frame, const CRect& size, void* parent, PlatformType parentType)
{
	return new HIViewFrame (frame, size, (WindowRef)parent);
}

#endif

SInt32 pSystemVersion;

//-----------------------------------------------------------------------------
bool isWindowComposited (WindowRef window)
{
	WindowAttributes attr;
	GetWindowAttributes (window, &attr);
	if (attr & kWindowCompositingAttribute)
		return true;
	return false;
}

//-----------------------------------------------------------------------------
static void CRect2Rect (const CRect &_cr, Rect &rr)
{
	CRect cr (_cr);
	cr.normalize ();
	rr.left   = (short)cr.left;
	rr.right  = (short)cr.right;
	rr.top    = (short)cr.top;
	rr.bottom = (short)cr.bottom;
}

//-----------------------------------------------------------------------------
static void Rect2CRect (Rect &rr, CRect &cr)
{
	cr.left   = rr.left;
	cr.right  = rr.right;
	cr.top    = rr.top;
	cr.bottom = rr.bottom;
}

static IDataPackage* gDragContainer = 0;

//-----------------------------------------------------------------------------
static CPoint GetMacDragMouse (HIViewFrame* frame, DragRef drag)
{
	HIViewRef view = frame->getPlatformControl ();
	CPoint where;
	Point r;
	if (GetDragMouse (drag, NULL, &r) == noErr)
	{
		HIPoint location;
		location = CGPointMake ((CGFloat)r.h, (CGFloat)r.v);
		HIPointConvert (&location, kHICoordSpaceScreenPixel, NULL, kHICoordSpaceView, view);
		where.x = (CCoord)location.x;
		where.y = (CCoord)location.y;
	}
	return where;
}


#if 0
//-----------------------------------------------------------------------------
// MacDragContainer Declaration
//-----------------------------------------------------------------------------
class MacDragContainer : public CDragContainer
{
public:
	MacDragContainer (DragRef platformDrag);
	~MacDragContainer ();

	virtual void* first (int32_t& size, int32_t& type);		///< returns pointer on a char array if type is known
	virtual void* next (int32_t& size, int32_t& type);		///< returns pointer on a char array if type is known
	
	virtual int32_t getType (int32_t idx) const;
	virtual int32_t getCount () const { return nbItems; }

	DragRef getPlatformDrag () const { return platformDrag; }

protected:
	DragRef platformDrag;
	PasteboardRef pasteboard;
	int32_t nbItems;
	
	int32_t iterator;
	void* lastItem;
};

static MacDragContainer* gDragContainer = 0;

//-----------------------------------------------------------------------------
static CPoint GetMacDragMouse (HIViewFrame* frame)
{
	HIViewRef view = frame->getPlatformControl ();
	CPoint where;
	Point r;
	if (GetDragMouse ((DragRef)gDragContainer->getPlatformDrag (), NULL, &r) == noErr)
	{
		HIPoint location;
		location = CGPointMake ((CGFloat)r.h, (CGFloat)r.v);
		HIPointConvert (&location, kHICoordSpaceScreenPixel, NULL, kHICoordSpaceView, view);
		where.x = (CCoord)location.x;
		where.y = (CCoord)location.y;
	}
	return where;
}

//-----------------------------------------------------------------------------
// MacDragContainer Implementation
//-----------------------------------------------------------------------------
MacDragContainer::MacDragContainer (DragRef inPlatformDrag)
: platformDrag (inPlatformDrag)
, pasteboard (0)
, nbItems (0)
, iterator (0)
, lastItem (0)
{
	if (GetDragPasteboard (inPlatformDrag, &pasteboard) == noErr)
	{
		ItemCount numItems;
		if (PasteboardGetItemCount (pasteboard, &numItems) == noErr)
			nbItems = numItems;
	}
}

//-----------------------------------------------------------------------------
MacDragContainer::~MacDragContainer ()
{
	if (lastItem)
	{
		std::free (lastItem);
		lastItem = 0;
	}
}

//-----------------------------------------------------------------------------
int32_t MacDragContainer::getType (int32_t idx) const
{
	if (platformDrag == 0)
		return CDragContainer::kError;

	PasteboardItemID itemID;
	if (PasteboardGetItemIdentifier (pasteboard, idx+1, &itemID) == noErr)
	{
		CFArrayRef flavors = 0;
		if (PasteboardCopyItemFlavors (pasteboard, itemID, &flavors) == noErr)
		{
			int32_t result = CDragContainer::kUnknown;
			for (CFIndex i = 0; i < CFArrayGetCount (flavors); i++)
			{
				CFStringRef flavorType = (CFStringRef)CFArrayGetValueAtIndex (flavors, i);
				if (flavorType == 0)
					continue;
				CFStringRef osTypeFlavorType = UTTypeCopyPreferredTagWithClass (flavorType, kUTTagClassOSType);
				if (osTypeFlavorType == 0)
					continue;
				if (CFStringCompare (osTypeFlavorType, CFSTR("utxt"), 0) == kCFCompareEqualTo)
					result = CDragContainer::kUnicodeText;
				else if (CFStringCompare (osTypeFlavorType, CFSTR("utf8"), 0) == kCFCompareEqualTo)
					result = CDragContainer::kUnicodeText;
				else if (CFStringCompare (osTypeFlavorType, CFSTR("furl"), 0) == kCFCompareEqualTo)
					result = CDragContainer::kFile;
				else if (CFStringCompare (osTypeFlavorType, CFSTR("TEXT"), 0) == kCFCompareEqualTo)
					result = CDragContainer::kText;
				else if (CFStringCompare (osTypeFlavorType, CFSTR("XML "), 0) == kCFCompareEqualTo)
					result = CDragContainer::kText;
				CFRelease (osTypeFlavorType);
				if (result != CDragContainer::kUnknown)
					break;
			}
			CFRelease (flavors);
			return result;
		}
	}
	return CDragContainer::kUnknown;
}

//-----------------------------------------------------------------------------
void* MacDragContainer::first (int32_t& size, int32_t& type)
{
	iterator = 0;
	return next (size, type);
}

//-----------------------------------------------------------------------------
void* MacDragContainer::next (int32_t& size, int32_t& type)
{
	if (platformDrag == 0)
	{
		type = CDragContainer::kError;
		return 0;
	}
	if (lastItem)
	{
		std::free (lastItem);
		lastItem = 0;
	}
	size = 0;
	type = CDragContainer::kUnknown;

	PasteboardItemID itemID;
	if (PasteboardGetItemIdentifier (pasteboard, ++iterator, &itemID) == noErr)
	{
		CFArrayRef flavors = 0;
		if (PasteboardCopyItemFlavors (pasteboard, itemID, &flavors) == noErr)
		{
			for (CFIndex i = 0; i < CFArrayGetCount (flavors); i++)
			{
				CFStringRef flavorType = (CFStringRef)CFArrayGetValueAtIndex (flavors, i);
				if (flavorType == 0)
					continue;
				CFStringRef osTypeFlavorType = UTTypeCopyPreferredTagWithClass (flavorType, kUTTagClassOSType);
				PasteboardFlavorFlags flavorFlags;
				PasteboardGetItemFlavorFlags (pasteboard, itemID, flavorType, &flavorFlags);
				CFDataRef flavorData = 0;
				if (PasteboardCopyItemFlavorData (pasteboard, itemID, flavorType, &flavorData) == noErr)
				{
					CFIndex flavorDataSize = CFDataGetLength (flavorData);
					const UInt8* data = CFDataGetBytePtr (flavorData);
					if (data)
					{
						if (osTypeFlavorType == 0)
						{
							type = CDragContainer::kUnknown;
							size = flavorDataSize;
							lastItem = std::malloc (size);
							memcpy (lastItem, data, size);
						}
						else if (CFStringCompare (osTypeFlavorType, CFSTR("utxt"), 0) == kCFCompareEqualTo)
						{
							CFStringRef utf16String = CFStringCreateWithBytes(0, data, flavorDataSize, kCFStringEncodingUTF16, false);
							if (utf16String)
							{
								CFIndex maxSize = CFStringGetMaximumSizeForEncoding (flavorDataSize/2, kCFStringEncodingUTF8);
								lastItem = std::malloc (maxSize+1);
								if (CFStringGetCString (utf16String, (char*)lastItem, maxSize, kCFStringEncodingUTF8))
								{
									type = CDragContainer::kUnicodeText;
									size = strlen ((const char*)lastItem);
								}
								else
								{
									std::free (lastItem);
									lastItem = 0;
								}
								CFRelease (utf16String);
							}
						}
						else if (CFStringCompare (osTypeFlavorType, CFSTR("furl"), 0) == kCFCompareEqualTo)
						{
							type = CDragContainer::kFile;
							CFURLRef url = CFURLCreateWithBytes (NULL, data, flavorDataSize, kCFStringEncodingUTF8, NULL);
							lastItem = std::malloc (PATH_MAX);
							CFURLGetFileSystemRepresentation (url, false, (UInt8*)lastItem, PATH_MAX);
							CFRelease (url);
							size = strlen ((const char*)lastItem);
						}
						else if (CFStringCompare (osTypeFlavorType, CFSTR("utf8"), 0) == kCFCompareEqualTo)
						{
							type = CDragContainer::kUnicodeText;
							size = flavorDataSize;
							lastItem = std::malloc (flavorDataSize + 1);
							((char*)lastItem)[flavorDataSize] = 0;
							memcpy (lastItem, data, flavorDataSize);
						}
						else if (CFStringCompare (osTypeFlavorType, CFSTR("TEXT"), 0) == kCFCompareEqualTo)
						{
							type = CDragContainer::kText;
							size = flavorDataSize;
							lastItem = std::malloc (flavorDataSize + 1);
							((char*)lastItem)[flavorDataSize] = 0;
							memcpy (lastItem, data, flavorDataSize);
						}
						else if (CFStringCompare (osTypeFlavorType, CFSTR("XML "), 0) == kCFCompareEqualTo)
						{
							type = CDragContainer::kText;
							size = flavorDataSize;
							lastItem = std::malloc (flavorDataSize + 1);
							((char*)lastItem)[flavorDataSize] = 0;
							memcpy (lastItem, data, flavorDataSize);
						}
					}
					CFRelease (flavorData);
				}
				if (osTypeFlavorType)
					CFRelease (osTypeFlavorType);
				if (type != CDragContainer::kError)
					break;
			}
			CFRelease (flavors);
			return lastItem;
		}
	}
	return NULL;
}
#endif

static bool addViewToContentView = true;
//-----------------------------------------------------------------------------
void HIViewFrame::setAddToContentView (bool addToContentView)
{
	addViewToContentView = addToContentView;
}

//-----------------------------------------------------------------------------
HIViewFrame::HIViewFrame (IPlatformFrameCallback* frame, const CRect& size, WindowRef parent)
: IPlatformFrame (frame)
, window (parent)
, controlRef (0)
, hasFocus (false)
, isInMouseTracking (false)
, mouseEventHandler (0)
{
	Gestalt (gestaltSystemVersion, &pSystemVersion);

	Rect r = {(short)size.top, (short)size.left, (short)size.bottom, (short)size.right};
	UInt32 features =	kControlSupportsDragAndDrop
						| kControlSupportsFocus
						| kControlHandlesTracking
						| kControlSupportsEmbedding
						| kHIViewIsOpaque
						| kHIViewFeatureDoesNotUseSpecialParts;
	OSStatus status = CreateUserPaneControl (0, &r, features, &controlRef);
	if (status != noErr)
	{
		fprintf (stderr, "Could not create Control : %d\n", (int32_t)status);
		return;
	}
	const EventTypeSpec controlEventTypes[] = {	
		{ kEventClassControl,	kEventControlDraw},
		{ kEventClassControl,	kEventControlHitTest},
		{ kEventClassControl,	kEventControlClick},
		//{kEventClassControl,	kEventControlTrack},
		//{kEventClassControl,	kEventControlContextualMenuClick},
		{ kEventClassKeyboard,	kEventRawKeyDown},
		{ kEventClassKeyboard,	kEventRawKeyRepeat},
		{ kEventClassMouse,		kEventMouseWheelMoved},
		{ kEventClassControl,	kEventControlDragEnter},
		{ kEventClassControl,	kEventControlDragWithin},
		{ kEventClassControl,	kEventControlDragLeave},
		{ kEventClassControl,	kEventControlDragReceive},
		{ kEventClassControl,	kEventControlInitialize},
		{ kEventClassControl,	kEventControlGetClickActivation},
		{ kEventClassControl,	kEventControlGetOptimalBounds},
		{ kEventClassScrollable,kEventScrollableGetInfo},
		{ kEventClassScrollable,kEventScrollableScrollTo},
		{ kEventClassControl,	kEventControlSetFocusPart},
		{ kEventClassControl,	kEventControlGetFocusPart},
		{ kEventClassControl,	kEventControlTrackingAreaExited},
	};
	InstallControlEventHandler (controlRef, carbonEventHandler, GetEventTypeCount (controlEventTypes), controlEventTypes, this, NULL);
	
	const EventTypeSpec keyWorkaroundEvents[] = {
		{ kEventClassTextInput, kEventTextInputUnicodeForKeyEvent },
		{ kEventClassWindow,	kEventWindowFocusAcquired },
		{ kEventClassWindow,	kEventWindowFocusRelinquish }
	};
	InstallWindowEventHandler (window, carbonEventHandler, GetEventTypeCount (keyWorkaroundEvents), keyWorkaroundEvents, this, &keyboardEventHandler);
	const EventTypeSpec mouseEvents[] = {
		{ kEventClassMouse, kEventMouseDown },
		{ kEventClassMouse, kEventMouseUp },
		{ kEventClassMouse, kEventMouseMoved },
		{ kEventClassMouse, kEventMouseDragged },
	};
	InstallWindowEventHandler (window, carbonMouseEventHandler, GetEventTypeCount (mouseEvents), mouseEvents, this, &mouseEventHandler);
	
	SetControlDragTrackingEnabled (controlRef, true);
	SetAutomaticControlDragTrackingEnabledForWindow (window, true);

	if (addViewToContentView)
	{
		if (isWindowComposited (window)) 
		{
			HIViewRef contentView;
			HIViewRef rootView = HIViewGetRoot (window);
			if (HIViewFindByID (rootView, kHIViewWindowContentID, &contentView) != noErr)
				contentView = rootView;
			HIViewAddSubview (contentView, controlRef);
		}
		else
		{
			ControlRef rootControl;
			GetRootControl (window, &rootControl);
			if (rootControl == NULL)
				CreateRootControl (window, &rootControl);
			EmbedControl(controlRef, rootControl);	
		}
	}
	
	HIViewTrackingAreaRef trackingAreaRef;	// will automatically removed if view is destroyed
	HIViewNewTrackingArea (controlRef, 0, 0, &trackingAreaRef);
	
	#if 0
	// TODO: currently this is not supported, do we need to ?
	size.offset (-size.left, -size.top);
	mouseableArea.offset (-size.left, -size.top);
	#endif
}

//-----------------------------------------------------------------------------
HIViewFrame::~HIViewFrame ()
{
	if (keyboardEventHandler)
		RemoveEventHandler (keyboardEventHandler);
	if (mouseEventHandler)
		RemoveEventHandler (mouseEventHandler);
	if (controlRef)
	{
		if (HIViewRemoveFromSuperview (controlRef) == noErr && isWindowComposited (window))
			CFRelease (controlRef);
	}
}

// IPlatformFrame
//-----------------------------------------------------------------------------
bool HIViewFrame::getGlobalPosition (CPoint& pos) const
{
	Rect bounds;
	GetWindowBounds (window, kWindowContentRgn, &bounds);
	
	CCoord x   = bounds.left;
	CCoord y   = bounds.top;

	if (isWindowComposited (window))
	{
		HIPoint hip = { 0.f, 0.f };
		HIViewRef contentView;
		HIViewFindByID (HIViewGetRoot (window), kHIViewWindowContentID, &contentView);
		if (contentView)
			HIViewConvertPoint (&hip, controlRef, contentView);
		x += (CCoord)hip.x;
		y += (CCoord)hip.y;
	}
	else
	{
		HIRect hirect;
		HIViewGetFrame (controlRef, &hirect);
		x += (CCoord)hirect.origin.x;
		y += (CCoord)hirect.origin.y;
	}
	x -= hiScrollOffset.x;
	y -= hiScrollOffset.y;

	pos.x = x;
	pos.y = y;
	return true;
}

//-----------------------------------------------------------------------------
bool HIViewFrame::setSize (const CRect& newSize)
{
	HIRect frameRect;
	HIViewGetFrame (controlRef, &frameRect);

#if MAC_OS_X_VERSION_MAX_ALLOWED <= MAC_OS_X_VERSION_10_6
	// keep old values
	CCoord oldWidth  = frameRect.size.width;
	CCoord oldHeight = frameRect.size.height;

	if (window)
	{
		if (!isWindowComposited (window))
		{
			Rect bounds;
			GetPortBounds (GetWindowPort (window), &bounds);
			SizeWindow (window, (short)((bounds.right - bounds.left) - oldWidth + newSize.getWidth ()),
									(short)((bounds.bottom - bounds.top) - oldHeight + newSize.getHeight ()), true);
		}
	}
#endif

	if (controlRef)
	{
		HIRect frameRect;
		HIViewGetFrame (controlRef, &frameRect);
		frameRect.origin.x = newSize.left;
		frameRect.origin.y = newSize.top;
		frameRect.size.width = newSize.getWidth ();
		frameRect.size.height = newSize.getHeight ();
		HIViewSetFrame (controlRef, &frameRect);
	}
	return true;
}

//-----------------------------------------------------------------------------
bool HIViewFrame::getSize (CRect& size) const
{
	HIRect hiRect;
	if (HIViewGetFrame (controlRef, &hiRect) == noErr)
	{
		size.left = (CCoord)hiRect.origin.x;
		size.top = (CCoord)hiRect.origin.y;
		size.setWidth ((CCoord)hiRect.size.width);
		size.setHeight ((CCoord)hiRect.size.height);
		return true;
	}
#if MAC_OS_X_VERSION_MAX_ALLOWED <= MAC_OS_X_VERSION_10_6
	Rect bounds;
	GetPortBounds (GetWindowPort (window), &bounds);

	size.left   = bounds.left;
	size.top    = bounds.top;
	size.right  = bounds.right;
	size.bottom = bounds.bottom;
	return true;
#else
	return false;
#endif
}

//-----------------------------------------------------------------------------
bool HIViewFrame::getCurrentMousePosition (CPoint& mousePosition) const
{
#if MAC_OS_X_VERSION_MAX_ALLOWED <= MAC_OS_X_VERSION_10_6
	// no up-to-date API call available for this, so use old QuickDraw
	Point p;
	CGrafPtr savedPort;
	Boolean portChanged = QDSwapPort (GetWindowPort (window), &savedPort);
	GetMouse (&p);
	if (portChanged)
		QDSwapPort (savedPort, NULL);
	mousePosition (p.h, p.v);
#endif

	HIPoint location;
	HIViewRef fromView = NULL;
	HIViewFindByID (HIViewGetRoot (window), kHIViewWindowContentID, &fromView);

#if MAC_OS_X_VERSION_MAX_ALLOWED > MAC_OS_X_VERSION_10_6
	HIGetMousePosition (kHICoordSpaceView, fromView, &location);
	location.x = floor (location.x + 0.5);
	location.y = floor (location.y + 0.5);
#else
	location = CGPointMake (mousePosition.x, mousePosition.y);
#endif

	HIPointConvert (&location, kHICoordSpaceView, fromView, kHICoordSpaceView, controlRef);
	mousePosition.x = (CCoord)location.x;
	mousePosition.y = (CCoord)location.y;

	return true;
}

//-----------------------------------------------------------------------------
bool HIViewFrame::getCurrentMouseButtons (CButtonState& buttons) const
{
	UInt32 state = GetCurrentButtonState ();
	if (state == kEventMouseButtonPrimary)
		buttons |= kLButton;
	if (state == kEventMouseButtonSecondary)
		buttons |= kRButton;
	if (state == kEventMouseButtonTertiary)
		buttons |= kMButton;
	if (state == 4)
		buttons |= kButton4;
	if (state == 5)
		buttons |= kButton5;

	state = GetCurrentKeyModifiers ();
	if (state & cmdKey)
		buttons |= kControl;
	if (state & shiftKey)
		buttons |= kShift;
	if (state & optionKey)
		buttons |= kAlt;
	if (state & controlKey)
		buttons |= kApple;
	// for the one buttons
	if (buttons & kApple && buttons & kLButton)
	{
		buttons &= ~(kApple | kLButton);
		buttons |= kRButton;
	}

	return true;
}

//-----------------------------------------------------------------------------
bool HIViewFrame::setMouseCursor (CCursorType type)
{
	switch (type)
	{
		case kCursorWait:
			SetThemeCursor (kThemeWatchCursor);
			break;
		case kCursorHSize:
			SetThemeCursor (kThemeResizeLeftRightCursor);
			break;
		case kCursorVSize:
			SetThemeCursor (kThemeResizeUpDownCursor);
			break;
		case kCursorNESWSize:
			SetThemeCursor (kThemeCrossCursor);
			break;
		case kCursorNWSESize:
			SetThemeCursor (kThemeCrossCursor);
			break;
		case kCursorSizeAll:
			SetThemeCursor (kThemeCrossCursor);
			break;
		case kCursorCopy:
			SetThemeCursor (kThemeCopyArrowCursor);
			break;
		case kCursorNotAllowed:
			SetThemeCursor (kThemeNotAllowedCursor);
			break;
		case kCursorHand:
			SetThemeCursor (kThemeOpenHandCursor);
			break;
		default:
			SetThemeCursor (kThemeArrowCursor);
			break;
	}
	return true;
}

//-----------------------------------------------------------------------------
bool HIViewFrame::invalidRect (const CRect& rect)
{
	if (isWindowComposited (window))
	{
		HIRect r = { {static_cast<CGFloat>(rect.left), static_cast<CGFloat>(rect.top)}, {static_cast<CGFloat>(rect.getWidth ()), static_cast<CGFloat>(rect.getHeight ())} };
		HIViewSetNeedsDisplayInRect (controlRef, &r, true);
	}
	else
	{
		HIRect hiRect;
		HIViewGetFrame (controlRef, &hiRect);
		CRect _rect (rect);
//		_rect.offset (size.left, size.top);
		_rect.offset ((CCoord)hiRect.origin.x, (CCoord)hiRect.origin.y);
		Rect r = {(short)_rect.top, (short)_rect.left, (short)_rect.bottom, (short)_rect.right};
		InvalWindowRect (window, &r);
	}
	return true;
}

//-----------------------------------------------------------------------------
bool HIViewFrame::scrollRect (const CRect& _src, const CPoint& distance)
{
	if (isWindowComposited (window))
	{
		CRect src (_src);
		if (distance.x > 0)
			src.right += distance.x;
		else if (distance.x < 0)
			src.left += distance.x;
		if (distance.y > 0)
			src.bottom += distance.y;
		else if (distance.y < 0)
			src.top += distance.y;
		CGRect cgRect = CGRectMake ((CGFloat)src.left, (CGFloat)src.top, (CGFloat)src.getWidth (), (CGFloat)src.getHeight ());
		if (HIViewScrollRect (controlRef, &cgRect, (CGFloat)distance.x, (CGFloat)distance.y) == noErr)
			return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
bool HIViewFrame::showTooltip (const CRect& _rect, const char* utf8Text)
{
	CRect rect (_rect);
	CPoint p;
	getGlobalPosition (p);
	rect.offset (p.x, p.y);

	HMHelpContentRec helpContent = {0};
	helpContent.version = 0;
	helpContent.absHotRect.left = rect.left;
	helpContent.absHotRect.right = rect.right;
	helpContent.absHotRect.top = rect.top;
	helpContent.absHotRect.bottom = rect.bottom;
	helpContent.tagSide = kHMDefaultSide;
	helpContent.content[0].contentType = kHMCFStringContent;
	helpContent.content[0].u.tagCFString = CFStringCreateWithCString (0, utf8Text, kCFStringEncodingUTF8);
	HMDisplayTag(&helpContent);
	CFRelease (helpContent.content[0].u.tagCFString);

	return true;
}

//-----------------------------------------------------------------------------
bool HIViewFrame::hideTooltip ()
{
	HMHideTag ();
	return true;
}

//-----------------------------------------------------------------------------
IPlatformTextEdit* HIViewFrame::createPlatformTextEdit (IPlatformTextEditCallback* textEdit)
{
	HIViewTextEdit* control = new HIViewTextEdit (controlRef, textEdit);
	if (control->getPlatformControl ())
		return control;
	control->forget ();
	return 0;
}

//-----------------------------------------------------------------------------
IPlatformOptionMenu* HIViewFrame::createPlatformOptionMenu ()
{
	return new HIViewOptionMenu ();
}

//-----------------------------------------------------------------------------
COffscreenContext* HIViewFrame::createOffscreenContext (CCoord width, CCoord height, double scaleFactor)
{
	CGBitmap* bitmap = new CGBitmap (CPoint (width * scaleFactor, height * scaleFactor));
	bitmap->setScaleFactor (scaleFactor);
	CGDrawContext* context = new CGDrawContext (bitmap);
	bitmap->forget ();
	if (context->getCGContext ())
		return context;
	context->forget ();
	return 0;
}

//------------------------------------------------------------------------------------
DragResult HIViewFrame::doDrag (IDataPackage* source, const CPoint& offset, CBitmap* dragBitmap)
{
	DragResult result = kDragError;
	PasteboardRef pb;
	if (PasteboardCreate (kPasteboardUniqueName, &pb) == noErr)
	{
		PasteboardClear (pb);
		for (uint32_t i = 0; i < source->getCount (); i++)
		{
			const void* buffer = 0;
			IDataPackage::Type type;
			int32_t bufferSize = source->getData (i, buffer, type);
			if (bufferSize > 0)
			{
				switch (type)
				{
					case IDataPackage::kFilePath:
					{
						CFURLRef cfUrl = CFURLCreateFromFileSystemRepresentation (0, (const UInt8*)buffer, bufferSize, false);
						if (cfUrl)
						{
							CFDataRef dataRef = CFURLCreateData (0, cfUrl, kCFStringEncodingUTF8, false);
							if (dataRef)
							{
								PasteboardPutItemFlavor (pb, (void*)buffer, kUTTypeFileURL, dataRef, kPasteboardFlavorNoFlags);
								CFRelease (dataRef);
							}
							CFRelease (cfUrl);
						}
						break;
					}
					case IDataPackage::kText:
					{
						CFStringRef stringRef = CFStringCreateWithCString (0, (const char*)buffer, kCFStringEncodingUTF8);
						if (stringRef)
						{
							CFDataRef dataRef = CFStringCreateExternalRepresentation (0, stringRef, kCFStringEncodingUTF8, 0);
							if (dataRef)
							{
								PasteboardPutItemFlavor (pb, (void*)buffer, kUTTypeUTF8PlainText, dataRef, kPasteboardFlavorNoFlags);
								CFRelease (dataRef);
							}
							CFRelease (stringRef);
						}
						break;
					}
					case IDataPackage::kBinary:
					{
						CFDataRef dataRef = CFDataCreate (0, (const UInt8*)buffer, bufferSize);
						if (dataRef)
						{
							PasteboardPutItemFlavor (pb, (void*)buffer, kUTTypeData, dataRef, kPasteboardFlavorSenderOnly);
							CFRelease (dataRef);
						}
						break;
					}
					case IDataPackage::kError:
						break;
				}
			}
		}
		DragRef drag;
		if (NewDragWithPasteboard (pb, &drag) == noErr)
		{
			EventRecord eventRecord;
			EventRef eventRef = GetCurrentEvent ();
			if (eventRef && ConvertEventRefToEventRecord (eventRef, &eventRecord))
			{

				CGBitmap* cgBitmap = dragBitmap ? dynamic_cast<CGBitmap*> (dragBitmap->getPlatformBitmap ()) : 0;
				CGImageRef cgImage = cgBitmap ? cgBitmap->getCGImage () : 0;
				if (cgImage)
				{
					HIPoint imageOffset = { static_cast<CGFloat>(offset.x), static_cast<CGFloat>(offset.y) };
					SetDragImageWithCGImage (drag, cgImage, &imageOffset, 0);
				}

				if (TrackDrag (drag, &eventRecord, NULL) == noErr)
				{
					DragActions action;
					if (GetDragDropAction (drag, &action) == noErr)
					{
						if (action == kDragActionNothing)
							result = kDragRefused;
						else if (action & kDragActionMove)
							result = kDragMoved;
						else
							result = kDragCopied;
					}
				}
			}
			DisposeDrag (drag);
		}
		CFRelease (pb);
	}
	return result;
}

//-----------------------------------------------------------------------------
void HIViewFrame::setClipboard (IDataPackage* data)
{
	MacClipboard::setClipboard (data);
}

//-----------------------------------------------------------------------------
IDataPackage* HIViewFrame::getClipboard ()
{
	return MacClipboard::createClipboardDataPackage ();
}


#define ENABLE_LOGGING 0

#if ENABLE_LOGGING
#define LOG_HIPOINT(text,point) fprintf (stdout, "%s%d, %d\n", text, (int32_t)point.x, (int32_t)point.y);
#define LOG(text) fprintf (stdout, "%s\n", text);
#else
#define LOG_HIPOINT(x,y)
#define LOG(x)
#endif

//-----------------------------------------------------------------------------
pascal OSStatus HIViewFrame::carbonMouseEventHandler (EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
{
	OSStatus result = eventNotHandledErr;
	HIViewFrame* hiviewframe = (HIViewFrame*)inUserData;
	IPlatformFrameCallback* frame = hiviewframe->frame;
	UInt32 eventClass = GetEventClass (inEvent);
	UInt32 eventKind = GetEventKind (inEvent);
	WindowRef window = (WindowRef)hiviewframe->window;
	HIViewRef hiView = hiviewframe->controlRef;

	HIViewRef view;
	if (HIViewGetViewForMouseEvent (HIViewGetRoot (window), inEvent, &view) == noErr)
	{
		if (view != hiView && !((eventKind == kEventMouseDragged || eventKind == kEventMouseUp) && hiviewframe->isInMouseTracking)) // TODO: make it work as above
			return result;
	}
	switch (eventClass)
	{
		case kEventClassMouse:
		{
			UInt32 modifiers = 0;
			EventMouseButton buttonState = 0;
			CButtonState buttons = 0;
			HIPoint location = { 0.f, 0.f };
			if (GetEventParameter (inEvent, kEventParamWindowMouseLocation, typeHIPoint, NULL, sizeof (HIPoint), NULL, &location) == noErr)
			{
				//LOG_HIPOINT("window :",location)
				HIPointConvert (&location, kHICoordSpaceWindow, window, kHICoordSpaceView, hiView);
				//LOG_HIPOINT("view   :",location)
			}
			if (!isWindowComposited ((WindowRef)window))
			{
				HIRect viewRect;
				HIViewGetFrame(hiView, &viewRect);
				location.x -= viewRect.origin.x;
				location.y -= viewRect.origin.y;
			}
			GetEventParameter (inEvent, kEventParamKeyModifiers, typeUInt32, NULL, sizeof (UInt32), NULL, &modifiers);
			GetEventParameter (inEvent, kEventParamMouseButton, typeMouseButton, NULL, sizeof (EventMouseButton), NULL, &buttonState);
			if (buttonState == kEventMouseButtonPrimary)
				buttons |= kLButton;
			if (buttonState == kEventMouseButtonSecondary)
				buttons |= kRButton;
			if (buttonState == kEventMouseButtonTertiary)
				buttons |= kMButton;
			if (buttonState == 4)
				buttons |= kButton4;
			if (buttonState == 5)
				buttons |= kButton5;
			if (modifiers & cmdKey)
				buttons |= kControl;
			if (modifiers & shiftKey)
				buttons |= kShift;
			if (modifiers & optionKey)
				buttons |= kAlt;
			if (modifiers & controlKey)
				buttons |= kApple;
			CPoint point ((CCoord)location.x, (CCoord)location.y);
			switch (eventKind)
			{
				case kEventMouseDown:
				{
					LOG("Mouse Down")
					UInt32 clickCount = 0;
					GetEventParameter (inEvent, kEventParamClickCount, typeUInt32, NULL, sizeof (UInt32), NULL, &clickCount);
					if (clickCount > 1)
						buttons |= kDoubleClick;
					result = CallNextEventHandler (inHandlerCallRef, inEvent); // calls default handler, which activates the window if not already active, or sets the process to front
					CMouseEventResult mer = frame->platformOnMouseDown (point, buttons);
					if (mer == kMouseEventHandled)
						hiviewframe->isInMouseTracking = true;
					if (mer != kMouseEventNotHandled)
						result = noErr;
					break;
				}
				case kEventMouseUp:
				{
					LOG("Mouse Up")
					if (frame->platformOnMouseUp (point, buttons) != kMouseEventNotHandled)
						result = noErr;
					hiviewframe->isInMouseTracking = false;
					break;
				}
				case kEventMouseDragged:
				{
					//LOG("Mouse Dragged")
					if (frame->platformOnMouseMoved (point, buttons) != kMouseEventNotHandled)
						result = noErr;
					break;
				}
				case kEventMouseMoved:
				{
					//LOG("Mouse Moved")
					if (IsWindowActive (window))
					{
						if (frame->platformOnMouseMoved (point, buttons) != kMouseEventNotHandled)
							result = noErr;
					}
					break;
				}
			}
			break;
		}
	}
	return result;
}

//------------------------------------------------------------------------------
static short keyTable[] = {
	VKEY_BACK,		0x33, 
	VKEY_TAB,		0x30, 
	VKEY_RETURN,	0x24, 
	VKEY_PAUSE,		0x71, 
	VKEY_ESCAPE,	0x35, 
	VKEY_SPACE,		0x31, 

	VKEY_END,		0x77, 
	VKEY_HOME,		0x73, 

	VKEY_LEFT,		0x7B, 
	VKEY_UP,		0x7E, 
	VKEY_RIGHT,		0x7C, 
	VKEY_DOWN,		0x7D, 
	VKEY_PAGEUP,	0x74, 
	VKEY_PAGEDOWN,	0x79, 

	VKEY_PRINT,		0x69, 			
	VKEY_ENTER,		0x4C, 
	VKEY_HELP,		0x72, 
	VKEY_DELETE,	0x75, 
	VKEY_NUMPAD0,	0x52, 
	VKEY_NUMPAD1,	0x53, 
	VKEY_NUMPAD2,	0x54, 
	VKEY_NUMPAD3,	0x55, 
	VKEY_NUMPAD4,	0x56, 
	VKEY_NUMPAD5,	0x57, 
	VKEY_NUMPAD6,	0x58, 
	VKEY_NUMPAD7,	0x59, 
	VKEY_NUMPAD8,	0x5B, 
	VKEY_NUMPAD9,	0x5C, 
	VKEY_MULTIPLY,	0x43, 
	VKEY_ADD,		0x45, 
	VKEY_SUBTRACT,	0x4E, 
	VKEY_DECIMAL,	0x41, 
	VKEY_DIVIDE,	0x4B, 
	VKEY_F1,		0x7A, 
	VKEY_F2,		0x78, 
	VKEY_F3,		0x63, 
	VKEY_F4,		0x76, 
	VKEY_F5,		0x60, 
	VKEY_F6,		0x61, 
	VKEY_F7,		0x62, 
	VKEY_F8,		0x64, 
	VKEY_F9,		0x65, 
	VKEY_F10,		0x6D, 
	VKEY_F11,		0x67, 
	VKEY_F12,		0x6F, 
	VKEY_NUMLOCK,	0x47, 
	VKEY_EQUALS,	0x51
};

/// @cond ignore
#if MAC_OS_X_VERSION_MAX_ALLOWED <= MAC_OS_X_VERSION_10_6
class VSTGUIDrawRectsHelper
{
public:
	VSTGUIDrawRectsHelper (CFrame* inFrame, CDrawContext* inContext, bool inIsComposited) : frame (inFrame), context (inContext), isComposited (inIsComposited) {}
	
	CFrame* frame;
	CDrawContext* context;
	bool isComposited;
};

static OSStatus VSTGUIDrawRectsProc (UInt16 message, RgnHandle rgn, const Rect* rect, void* refCon)
{
	if (message == kQDRegionToRectsMsgParse)
	{
		VSTGUIDrawRectsHelper* h = (VSTGUIDrawRectsHelper*)refCon;
		CRect r;
		Rect2CRect ((Rect&)*rect, r);
//		if (!h->isComposited)
//			r.offset (-h->context->offsetScreen.x, -h->context->offsetScreen.y);
		h->context->saveGlobalState ();
		h->frame->drawRect (h->context, r);
		h->context->restoreGlobalState ();
	}
	return noErr;
}
#else
struct CFrameAndCDrawContext {
	CFrame* frame;
	CDrawContext* context;
	
	CFrameAndCDrawContext (CFrame* f, CDrawContext* c) : frame (f), context (c) {}
};

static OSStatus HIShapeEnumerateProc (int inMessage, HIShapeRef inShape, const CGRect *inRect, void *inRefcon)
{
	if (inMessage == kHIShapeEnumerateRect)
	{
		CFrameAndCDrawContext* tmp = (CFrameAndCDrawContext*)inRefcon;
		CRect r (inRect->origin.x, inRect->origin.y);
		r.setWidth (inRect->size.width);
		r.setHeight (inRect->size.height);
		tmp->context->saveGlobalState ();
		tmp->frame->drawRect (tmp->context, r);
		tmp->context->restoreGlobalState ();
	}
	return noErr;
}

#endif
/// @endcond

#ifndef kHIViewFeatureGetsFocusOnClick
#define   kHIViewFeatureGetsFocusOnClick (1 << 8)
#endif

bool hiToolboxAllowFocusChange = true;

//-----------------------------------------------------------------------------
pascal OSStatus HIViewFrame::carbonEventHandler (EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
{
	OSStatus result = eventNotHandledErr;
	HIViewFrame* hiviewframe = (HIViewFrame*)inUserData;
	IPlatformFrameCallback* frame = hiviewframe->frame;
	UInt32 eventClass = GetEventClass (inEvent);
	UInt32 eventKind = GetEventKind (inEvent);
	WindowRef window = hiviewframe->window;

	switch (eventClass)
	{
		#if 0 // TODO: make it work, or remove support for embedding the frame into a native scroll view
		case kEventClassScrollable:
		{
			switch (eventKind)
			{
				case kEventScrollableGetInfo:
				{
					HISize cs = {frame->getWidth (), frame->getHeight ()};
					SetEventParameter (inEvent, kEventParamImageSize, typeHISize, sizeof (HISize), &cs);
					HIPoint origin = {frame->hiScrollOffset.x, frame->hiScrollOffset.y};
					SetEventParameter (inEvent, kEventParamOrigin, typeHIPoint, sizeof (HIPoint), &origin);
					HISize lineSize = {50.0, 20.0};
					SetEventParameter(inEvent, kEventParamLineSize, typeHISize, sizeof(lineSize), &lineSize);
					HIRect bounds;
					HIViewGetBounds (hiviewframe->controlRef, &bounds);
					SetEventParameter(inEvent, kEventParamViewSize, typeHISize, sizeof(bounds.size), &bounds.size);
					result = noErr;
					break;
				}
				case kEventScrollableScrollTo:
				{
					HIPoint where;
					GetEventParameter(inEvent, kEventParamOrigin, typeHIPoint, NULL, sizeof(where), NULL, &where);
					frame->hiScrollOffset.x = (CCoord)where.x;
					frame->hiScrollOffset.y = (CCoord)where.y;
					HIViewSetBoundsOrigin(hiviewframe->controlRef, where.x, where.y);
					HIViewSetNeedsDisplay(hiviewframe->controlRef, true);
					result = noErr;
					break;
				}
			}
			break;
		}
		#endif
		case kEventClassControl:
		{
			switch (eventKind)
			{
				case kEventControlInitialize:
				{
					UInt32 controlFeatures = kControlSupportsDragAndDrop | kControlSupportsFocus | kControlHandlesTracking | kControlSupportsEmbedding | kHIViewFeatureGetsFocusOnClick | kHIViewIsOpaque | kHIViewGetsFocusOnClick;
					SetEventParameter (inEvent, kEventParamControlFeatures, typeUInt32, sizeof (UInt32), &controlFeatures);
					result = noErr;
					break;
				}
				case kEventControlDraw:
				{
					CDrawContext* context = 0;
					HIRect controlBounds;
					HIViewGetBounds (hiviewframe->controlRef, &controlBounds);
					CRect dirtyRect;
					dirtyRect.left = controlBounds.origin.x;
					dirtyRect.top = controlBounds.origin.y;
					dirtyRect.setWidth (controlBounds.size.width);
					dirtyRect.setHeight (controlBounds.size.height);
					CGContextRef cgcontext = 0;
					OSStatus res = GetEventParameter (inEvent, kEventParamCGContextRef, typeCGContextRef, NULL, sizeof (cgcontext), NULL, &cgcontext);
					if (res != noErr)
						break;
					context = new CGDrawContext (cgcontext, dirtyRect);
					context->beginDraw ();

				#if MAC_OS_X_VERSION_MAX_ALLOWED <= MAC_OS_X_VERSION_10_6
					RgnHandle dirtyRegion;
					if (GetEventParameter (inEvent, kEventParamRgnHandle, typeQDRgnHandle, NULL, sizeof (RgnHandle), NULL, &dirtyRegion) == noErr)
					{
						CFrame* cframe = dynamic_cast<CFrame*> (frame);
						VSTGUIDrawRectsHelper helper (cframe, context, isWindowComposited (window));
						RegionToRectsUPP upp = NewRegionToRectsUPP (VSTGUIDrawRectsProc);
						QDRegionToRects (dirtyRegion, kQDParseRegionFromTopLeft, upp, &helper);
						DisposeRegionToRectsUPP (upp);
					}
					else
				#else
					HIShapeRef shapeRef;
					if (GetEventParameter (inEvent, kEventParamShape, typeHIShapeRef, NULL, sizeof (HIShapeRef), NULL, &shapeRef) == noErr)
					{
						CFrame* cframe = dynamic_cast<CFrame*> (frame);
						CFrameAndCDrawContext tmp (cframe, context);
						HIShapeEnumerate (shapeRef, kHIShapeParseFromTopLeft, HIShapeEnumerateProc, &tmp);
					}
				#endif
					{
						frame->platformDrawRect (context, dirtyRect);
					}
					context->endDraw ();
					context->forget ();
					result = noErr;
					break;
				}
				case kEventControlGetClickActivation:
				{
					ClickActivationResult activation = kActivateAndHandleClick;
					SetEventParameter (inEvent, kEventParamClickActivation, typeClickActivationResult, sizeof (ClickActivationResult), &activation);
					result = noErr;
					break;
				}
				case kEventControlHitTest:
				{
					ControlPartCode code = kControlContentMetaPart;
					SetEventParameter (inEvent, kEventParamControlPart, typeControlPartCode, sizeof (ControlPartCode), &code);
					result = noErr;
					break;
				}
				case kEventControlClick:
				{
					return noErr;
					EventMouseButton buttonState;
					GetEventParameter (inEvent, kEventParamMouseButton, typeMouseButton, NULL, sizeof (EventMouseButton), NULL, &buttonState);
					if (buttonState == kEventMouseButtonPrimary)
					{
						result = CallNextEventHandler (inHandlerCallRef, inEvent);
						break;
					}
				}
				case kEventControlTrack:
				case kEventControlContextualMenuClick:
				{
					break;
				}
				#if 0 // TODO: is this necessary
				case kEventControlGetOptimalBounds:
				{
					HIRect optimalBounds = { {0, 0}, { frame->getWidth (), frame->getHeight ()}};
					SetEventParameter (inEvent, kEventParamControlOptimalBounds, typeHIRect, sizeof (HIRect), &optimalBounds);
					result = noErr;
					break;
				}
				#endif
				case kEventControlGetFocusPart:
				{
					if (hiToolboxAllowFocusChange)
					{
						ControlPartCode code = hiviewframe->hasFocus ? 127 : kControlFocusNoPart;
						SetEventParameter (inEvent, kEventParamControlPart, typeControlPartCode, sizeof (ControlPartCode), &code);
						result = noErr;
					}
					break;
				}
				case kEventControlSetFocusPart:
				{
					if (hiToolboxAllowFocusChange)
					{
						CFrame* cframe = dynamic_cast<CFrame*> (frame);
						if (cframe)
						{
							ControlPartCode code;
							GetEventParameter (inEvent, kEventParamControlPart, typeControlPartCode, NULL, sizeof (ControlPartCode), NULL, &code);
							if (code == kControlFocusNoPart)
							{
								hiviewframe->hasFocus = false;
							}
							else
							{
								bool anfResult = false;
								if (code == kControlFocusNextPart)
									anfResult = cframe->advanceNextFocusView (cframe->getFocusView ());
								else if (code == kControlFocusPrevPart)
									anfResult = cframe->advanceNextFocusView (cframe->getFocusView (), true);
								if (anfResult)
								{
									hiviewframe->hasFocus = true;
									code = 127;
								}
								else
								{
									hiviewframe->hasFocus = false;
									code = kControlFocusNoPart;
								}
								SetEventParameter (inEvent, kEventParamControlPart, typeControlPartCode, sizeof (code), &code);
							}
							result = noErr;
						}
					}
					break;
				}
				case kEventControlDragEnter:
				{
					DragRef dragRef;
					if (GetEventParameter (inEvent, kEventParamDragRef, typeDragRef, NULL, sizeof (DragRef), NULL, &dragRef) == noErr)
					{
						gDragContainer = MacClipboard::createCarbonDragDataPackage (dragRef);
						
						CPoint where = GetMacDragMouse (hiviewframe, dragRef);
						hiviewframe->setMouseCursor (kCursorNotAllowed);
						frame->platformOnDragEnter (gDragContainer, where);

						Boolean acceptDrop = true;
						SetEventParameter (inEvent, kEventParamControlWouldAcceptDrop, typeBoolean, sizeof (Boolean), &acceptDrop);
					}
					result = noErr;
					break;
				}
				case kEventControlDragWithin:
				{
					DragRef dragRef;
					if (gDragContainer && GetEventParameter (inEvent, kEventParamDragRef, typeDragRef, NULL, sizeof (DragRef), NULL, &dragRef) == noErr)
					{
						CPoint where = GetMacDragMouse (hiviewframe, dragRef);
						frame->platformOnDragMove (gDragContainer, where);
					}
					result = noErr;
					break;
				}
				case kEventControlDragLeave:
				{
					DragRef dragRef;
					if (gDragContainer && GetEventParameter (inEvent, kEventParamDragRef, typeDragRef, NULL, sizeof (DragRef), NULL, &dragRef) == noErr)
					{
						CPoint where = GetMacDragMouse (hiviewframe, dragRef);
						frame->platformOnDragLeave (gDragContainer, where);
						hiviewframe->setMouseCursor (kCursorDefault);
					}
					result = noErr;
					break;
				}
				case kEventControlDragReceive:
				{
					DragRef dragRef;
					if (gDragContainer && GetEventParameter (inEvent, kEventParamDragRef, typeDragRef, NULL, sizeof (DragRef), NULL, &dragRef) == noErr)
					{
						CPoint where = GetMacDragMouse (hiviewframe, dragRef);
						frame->platformOnDrop (gDragContainer, where);
						hiviewframe->setMouseCursor (kCursorDefault);
						gDragContainer->forget ();
						gDragContainer = 0;
					}
					result = noErr;
					break;
				}
				case kEventControlTrackingAreaExited:
				{
					HIPoint location = { 0.f, 0.f };
					if (GetEventParameter (inEvent, kEventParamMouseLocation, typeHIPoint, NULL, sizeof (HIPoint), NULL, &location) == noErr)
					{
						if (!isWindowComposited (window))
						{
							HIRect viewRect;
							HIViewGetFrame (hiviewframe->controlRef, &viewRect);
							location.x -= viewRect.origin.x;
							location.y -= viewRect.origin.y;
						}
						CPoint point ((CCoord)location.x, (CCoord)location.y);
						frame->platformOnMouseExited (point, 0);
					}
					break;
				}
			}
			break;
		}
		case kEventClassMouse:
		{
			switch (eventKind)
			{
				case kEventMouseWheelMoved:
				{
					UInt32 modifiers;
					HIPoint windowHIPoint;
					SInt32 wheelDelta;
					EventMouseWheelAxis wheelAxis;
					WindowRef windowRef;
					GetEventParameter (inEvent, kEventParamWindowRef, typeWindowRef, NULL, sizeof (WindowRef), NULL, &windowRef);
					GetEventParameter (inEvent, kEventParamMouseWheelAxis, typeMouseWheelAxis, NULL, sizeof (EventMouseWheelAxis), NULL, &wheelAxis);
					GetEventParameter (inEvent, kEventParamMouseWheelDelta, typeSInt32, NULL, sizeof (SInt32), NULL, &wheelDelta);
					GetEventParameter (inEvent, kEventParamWindowMouseLocation, typeHIPoint, NULL, sizeof (HIPoint), NULL, &windowHIPoint);
					GetEventParameter (inEvent, kEventParamKeyModifiers, typeUInt32, NULL, sizeof (UInt32), NULL, &modifiers);
					CButtonState buttons = 0;
					if (modifiers & cmdKey)
						buttons |= kControl;
					if (modifiers & shiftKey)
						buttons |= kShift;
					if (modifiers & optionKey)
						buttons |= kAlt;
					if (modifiers & controlKey)
						buttons |= kApple;
					
					HIPointConvert (&windowHIPoint, kHICoordSpaceWindow, windowRef, kHICoordSpaceView, hiviewframe->controlRef);
					
					// non-compositing window controls need to handle offset themselves
					if (!isWindowComposited (windowRef))
					{
						HIRect viewRect;
						HIViewGetFrame(hiviewframe->controlRef, &viewRect);
						windowHIPoint.x -= viewRect.origin.x;
						windowHIPoint.y -= viewRect.origin.y;
					}
					
					CPoint p ((CCoord)windowHIPoint.x, (CCoord)windowHIPoint.y);
					float distance = wheelDelta;
					CMouseWheelAxis axis = kMouseWheelAxisY;
					if (wheelAxis == kEventMouseWheelAxisX)
						axis = kMouseWheelAxisX;
					frame->platformOnMouseWheel (p, axis, distance, buttons);
					result = noErr;
					break;
				}
			}
			break;
		}
		case kEventClassTextInput:
		{
			switch (eventKind)
			{
				case kEventTextInputUnicodeForKeyEvent:
				{
					// The "Standard Event Handler" of a window would return noErr even though no one has handled the key event. 
					// This prevents the "Standard Handler" to be called for this event, with the exception of the tab key as it is used for control focus changes.
					EventRef rawKeyEvent;
					GetEventParameter (inEvent, kEventParamTextInputSendKeyboardEvent, typeEventRef, NULL, sizeof (EventRef), NULL, &rawKeyEvent);
					if (rawKeyEvent)
					{
						UInt32 keyCode = 0;
						GetEventParameter (rawKeyEvent, kEventParamKeyCode, typeUInt32, NULL, sizeof (UInt32), NULL, &keyCode);
						if (keyCode == (UInt32)keyTable[VKEY_TAB+1])
							return result;
					}
					result = eventPassToNextTargetErr;
					break;
				}
			}
			break;
		}
		case kEventClassWindow:
		{
			switch (eventKind)
			{
				case kEventWindowFocusAcquired:
				{
					frame->platformOnActivate (true);
					break;
				}
				case kEventWindowFocusRelinquish:
				{
					frame->platformOnActivate (false);
					break;
				}
			}
			break;
		}
		case kEventClassKeyboard:
		{
			if (hiviewframe->hasFocus)
			{
				switch (eventKind)
				{
					case kEventRawKeyDown:
					case kEventRawKeyRepeat:
					{
						// todo: make this work

						char character = 0;
						UInt32 keyCode = 0;
						UInt32 modifiers = 0;
						GetEventParameter (inEvent, kEventParamKeyMacCharCodes, typeChar, NULL, sizeof (char), NULL, &character);
						GetEventParameter (inEvent, kEventParamKeyCode, typeUInt32, NULL, sizeof (UInt32), NULL, &keyCode);
						GetEventParameter (inEvent, kEventParamKeyModifiers, typeUInt32, NULL, sizeof (UInt32), NULL, &modifiers);
						char scanCode = keyCode;
						VstKeyCode vstKeyCode;
						memset (&vstKeyCode, 0, sizeof (VstKeyCode));
						KeyboardLayoutRef layout;
						if (KLGetCurrentKeyboardLayout (&layout) == noErr)
						{
							const void* pKCHR = 0;
							KLGetKeyboardLayoutProperty (layout, kKLKCHRData, &pKCHR);
							if (pKCHR)
							{
								static UInt32 keyTranslateState = 0;
								vstKeyCode.character = KeyTranslate (pKCHR, keyCode, &keyTranslateState);
								if (modifiers & shiftKey)
								{
									vstKeyCode.character = toupper (vstKeyCode.character);
								}
							}
						}
						short entries = sizeof (keyTable) / (sizeof (short));
						for (int32_t i = 0; i < entries; i += 2)
						{
							if (keyTable[i + 1] == scanCode)
							{
								vstKeyCode.virt = keyTable[i];
								vstKeyCode.character = 0;
								break;
							}
						}
						if (modifiers & cmdKey)
							vstKeyCode.modifier |= MODIFIER_CONTROL;
						if (modifiers & shiftKey)
							vstKeyCode.modifier |= MODIFIER_SHIFT;
						if (modifiers & optionKey)
							vstKeyCode.modifier |= MODIFIER_ALTERNATE;
						if (modifiers & controlKey)
							vstKeyCode.modifier |= MODIFIER_COMMAND;
						if (frame->platformOnKeyDown (vstKeyCode))
							result = noErr;
						
						break;
					}
				}
			}
			break;
		}
	}
	return result;
}


} // namespace

#pragma GCC diagnostic warning "-Wdeprecated-declarations" // we know that we use deprecated functions from Carbon, so we don't want to be warned

#endif // MAC_CARBON
