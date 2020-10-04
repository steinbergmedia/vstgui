// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "vstguibase.h"
#include <functional>
#include <memory>

namespace VSTGUI {

//-----------------------------------------------------------------------------
using CViewAttributeID = size_t;
using ModalViewSessionID = uint32_t;

//-----------------------------------------------------------------------------
static constexpr uint32_t kStreamIOError = std::numeric_limits<uint32_t>::max ();
static constexpr int64_t kStreamSeekError = -1;

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
	/** arrow cursor */
	kCursorDefault = 0,
	/** wait cursor */
	kCursorWait,
	/** horizontal size cursor */
	kCursorHSize,
	/** vertical size cursor */
	kCursorVSize,
	/** size all cursor */
	kCursorSizeAll,
	/** northeast and southwest size cursor */
	kCursorNESWSize,
	/** northwest and southeast size cursor */
	kCursorNWSESize,
	/** copy cursor (mainly for drag&drop operations) */
	kCursorCopy,
	/** not allowed cursor (mainly for drag&drop operations) */
	kCursorNotAllowed,
	/** hand cursor */
	kCursorHand,
	/** i beam cursor */
	kCursorIBeam,
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
	kAutosizeColumn			= 1 << 4,
	kAutosizeRow			= 1 << 5,
	kAutosizeAll			= kAutosizeLeft | kAutosizeTop | kAutosizeRight | kAutosizeBottom
};

enum DragResult {
	kDragRefused = 0,
	kDragMoved,
	kDragCopied,
	kDragError = -1
};

//----------------------------
// @brief Bitmap Interpolation
//----------------------------
enum class BitmapInterpolationQuality
{
	kDefault = 0,	///< Let system decide
	kLow,			///< Nearest neighbour
	kMedium,		///< Bilinear interpolation
	kHigh			///< Bicubic interpolation (Bilinear on Windows)
};

//-----------------------------------------------------------------------------
enum class CSliderMode
{
	Touch,
	RelativeTouch,
	FreeClick,
	Ramp,
	UseGlobal
};

enum class DragOperation;

// @brief Stream seek modes
enum class SeekMode
{
	Set,
	Current,
	End
};

// simple structs
struct CColor;
struct CPoint;
struct CRect;
struct CButtonState;
struct CDrawMode;
struct CGraphicsTransform;
struct DragDescription;
struct DragEventData;
struct ModalViewSession;
struct CListControlRowDesc;
struct CNinePartTiledDescription;

// interfaces
class IViewListener;
class IViewContainerListener;
class IViewMouseListener;
class IDataPackage;
class IDependency;
class IFocusDrawing;
class IScaleFactorChangedListener;
class IDataBrowserDelegate;
class IMouseObserver;
class IKeyboardHook;
class IViewAddedRemovedObserver;
class IFocusViewObserver;
class ISplitViewController;
class ISplitViewSeparatorDrawer;
class IScrollbarDrawer;
class IControlListener;
class IDragCallback;
class IDraggingSession;
class IDropTarget;
class ICommandMenuItemTarget;
class IOptionMenuListener;
class ITextLabelListener;
class IListControlDrawer;
class IListControlConfigurator;

#if VSTGUI_TOUCH_EVENT_HANDLING
class ITouchEvent;
#endif

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
class StaticListControlConfigurator;
class StringListControlDrawer;

using CFontRef = CFontDesc*;

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
class CListControl;

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
class IPlatformResourceInputStream;

class IPlatformFactory;
class IPlatformFrame;
class IPlatformBitmap;
class IPlatformFont;
class IPlatformString;
class IPlatformTimer;
class IPlatformResourceInputStream;
class IPlatformFrameConfig;
class IPlatformFrameCallback;
class IPlatformTimerCallback;

enum class PlatformType : int32_t;

using PlatformFramePtr = SharedPointer<IPlatformFrame>;
using PlatformBitmapPtr = SharedPointer<IPlatformBitmap>;
using PlatformFontPtr = SharedPointer<IPlatformFont>;
using PlatformStringPtr = SharedPointer<IPlatformString>;
using PlatformTimerPtr = SharedPointer<IPlatformTimer>;
using PlatformResourceInputStreamPtr = std::unique_ptr<IPlatformResourceInputStream>;
using PlatformFactoryPtr = std::unique_ptr<IPlatformFactory>;


} // VSTGUI
