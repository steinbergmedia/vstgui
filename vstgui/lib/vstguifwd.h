// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __vstguifwd__
#define __vstguifwd__

#include "vstguibase.h"
#include <functional>

namespace VSTGUI {

//-----------------------------------------------------------------------------
using CViewAttributeID = size_t;

// enums
//----------------------------
// @brief Mouse Wheel Axis
//----------------------------
enum CMouseWheelAxis
{
	kMouseWheelAxisX = 0,
	kMouseWheelAxisY
};

//----------------------------
// @brief Mouse Event Results
//----------------------------
enum CMouseEventResult
{
	kMouseEventNotImplemented = 0,
	kMouseEventHandled,
	kMouseEventNotHandled,
	kMouseDownEventHandledButDontNeedMovedOrUpEvents,
	kMouseMoveEventHandledButDontNeedMoreEvents
};

//----------------------------
// @brief Cursor Type
//----------------------------
enum CCursorType
{
	kCursorDefault = 0,				///< arrow cursor
	kCursorWait,					///< wait cursor
	kCursorHSize,					///< horizontal size cursor
	kCursorVSize,					///< vertical size cursor
	kCursorSizeAll,					///< size all cursor
	kCursorNESWSize,				///< northeast and southwest size cursor
	kCursorNWSESize,				///< northwest and southeast size cursor
	kCursorCopy,					///< copy cursor (mainly for drag&drop operations)
	kCursorNotAllowed,				///< not allowed cursor (mainly for drag&drop operations)
	kCursorHand						///< hand cursor
};

//----------------------------
// @brief View Autosizing
//----------------------------
enum CViewAutosizing
{
	kAutosizeNone			= 0,
	kAutosizeLeft			= 1 << 0,
	kAutosizeTop			= 1 << 1,
	kAutosizeRight			= 1 << 2,
	kAutosizeBottom			= 1 << 3,
	kAutosizeColumn			= 1 << 4,	///< view containers treat their children as columns
	kAutosizeRow			= 1 << 5,	///< view containers treat their children as rows
	kAutosizeAll			= kAutosizeLeft | kAutosizeTop | kAutosizeRight | kAutosizeBottom
};

enum DragResult {
	kDragRefused = 0,
	kDragMoved,
	kDragCopied,
	kDragError = -1
};

// simple structs
struct CColor;
struct CPoint;
struct CRect;
struct CButtonState;
struct CDrawMode;
struct CGraphicsTransform;

// interfaces
class IViewListener;
class IViewContainerListener;
class IDataPackage;
class IDependency;
class IFocusDrawing;
class IScaleFactorChangedListener;
#if VSTGUI_TOUCH_EVENT_HANDLING
class ITouchEvent;
#endif
class IDataBrowserDelegate;
class IMouseObserver;
class IKeyboardHook;
class IViewAddedRemovedObserver;
class IFocusViewObserver;
class ISplitViewController;
class ISplitViewSeparatorDrawer;
class IScrollbarDrawer;
class IControlListener;

// classes
class CBitmap;
class CNinePartTiledBitmap;
class CResourceDescription;
class CLineStyle;
class CDrawContext;
class COffscreenContext;
class CDropSource;
class CFileExtension;
class CNewFileSelector;
class CFontDesc;
class VSTGUIEditorInterface;
class CTooltipSupport;
class CGraphicsPath;
class CGradient;
class UTF8String;
class UTF8StringView;
class CVSTGUITimer;
class CMenuItem;
class CCommandMenuItem;
class GenericStringListDataBrowserSource;

// views
class CFrame;
class CDataBrowser;
class CGradientView;
class CLayeredViewContainer;
class CAutoLayoutContainerView;
class CRowColumnView;
class CScrollView;
class CShadowViewContainer;
class CSplitView;
class CTabView;
class CView;
class CViewContainer;
#if VSTGUI_OPENGL_SUPPORT
class COpenGLView;
#endif

// controls
class CAutoAnimation;
class COnOffButton;
class CCheckBox;
class CKickButton;
class CTextButton;
class CColorChooser;
class CControl;
class CFontChooser;
class CKnob;
class CAnimKnob;
class CMovieBitmap;
class CMovieButton;
class COptionMenu;
class CParamDisplay;
class CScrollbar;
class CSegmentButton;
class CSlider;
class CVerticalSlider;
class CHorizontalSlider;
class CSpecialDigit;
class CSplashScreen;
class CAnimationSplashScreen;
class CVerticalSwitch;
class CHorizontalSwitch;
class CRockerSwitch;
class CTextEdit;
class CSearchTextEdit;
class CTextLabel;
class CMultiLineTextLabel;
class CVuMeter;
class CXYPad;

// animation
namespace Animation {
class IAnimationTarget;
class ITimingFunction;
class AlphaValueAnimation;
class ViewSizeAnimation;
class ExchangeViewAnimation;
class ControlValueAnimation;
class Animator;
class TimingFunctionBase;
class LinearTimingFunction;
class PowerTimingFunction;
class InterpolationTimingFunction;
class RepeatTimingFunction;
using DoneFunction = std::function<void (CView*, const IdStringPtr, IAnimationTarget*)>;
} // Animation

template <class I> class SharedPointer;

// platform
class IPlatformTextEdit;
class IPlatformTextEditCallback;
class IPlatformOptionMenu;
class IPlatformOpenGLView;
class IPlatformViewLayer;
class IPlatformViewLayerDelegate;
class IPlatformString;
class IPlatformBitmap;
class IPlatformBitmapPixelAccess;
class IPlatformFont;
class IPlatformFrame;
class IFontPainter;

}

#endif // __vstguifwd__
