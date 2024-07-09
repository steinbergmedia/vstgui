// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "vstguibase.h"
#include <functional>
#include <limits>
#include <memory>
#include <map>
#include <vector>

namespace VSTGUI {

//-----------------------------------------------------------------------------
using CViewAttributeID = size_t;
using ModalViewSessionID = uint32_t;

//-----------------------------------------------------------------------------
static constexpr uint32_t kStreamIOError = (std::numeric_limits<uint32_t>::max) ();
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
	/** crosshair cursor */
	kCursorCrosshair,
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

//-----------
// @brief Text Face
//-----------
enum CTxtFace
{
	kNormalFace = 0,
	kBoldFace = 1 << 1,
	kItalicFace = 1 << 2,
	kUnderlineFace = 1 << 3,
	kStrikethroughFace = 1 << 4
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
struct CMultiFrameBitmapDescription;

using GradientColorStop = std::pair<double, CColor>;
using GradientColorStopMap = std::multimap<double, CColor>;

using LinePair = std::pair<CPoint, CPoint>;
using LineList = std::vector<LinePair>;
using PointList = std::vector<CPoint>;

// interfaces
class IViewListener;
class IViewEventListener;
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
class ITextEditListener;
class ITextLabelListener;
class IListControlDrawer;
class IListControlConfigurator;

#if VSTGUI_TOUCH_EVENT_HANDLING
class ITouchEvent;
#endif

// classes
class CBitmap;
class CMultiFrameBitmap;
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

// events
struct Event;
struct ModifierEvent;
struct MousePositionEvent;
struct MouseEvent;
struct MouseDownUpMoveEvent;
struct MouseDownEvent;
struct MouseMoveEvent;
struct MouseUpEvent;
struct MouseCancelEvent;
struct MouseEnterEvent;
struct MouseExitEvent;
struct GestureEvent;
struct MouseWheelEvent;
struct ZoomGestureEvent;
struct KeyboardEvent;
struct Modifiers;
enum class EventType : uint32_t;
enum class VirtualKey : uint32_t;
enum class ModifierKey : uint32_t;

const Event& noEvent ();

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
class IPlatformGradient;
class IPlatformGraphicsPath;
class IPlatformGraphicsPathFactory;
class IPlatformString;
class IPlatformTimer;
class IPlatformResourceInputStream;
class IPlatformFrameConfig;
class IPlatformFrameCallback;
class IPlatformTimerCallback;
class IPlatformFileSelector;
class IPlatformGraphicsDeviceFactory;
class IPlatformGraphicsDevice;
class IPlatformGraphicsDeviceContext;
class IPlatformGraphicsDeviceContextBitmapExt;

struct PlatformFileExtension;
struct PlatformFileSelectorConfig;

enum class PlatformType : int32_t;
enum class PlatformGraphicsPathFillMode : int32_t;
enum class PlatformFileSelectorStyle : uint32_t;
enum class PlatformFileSelectorFlags : uint32_t;

using PlatformFramePtr = SharedPointer<IPlatformFrame>;
using PlatformBitmapPtr = SharedPointer<IPlatformBitmap>;
using PlatformFontPtr = SharedPointer<IPlatformFont>;
using PlatformStringPtr = SharedPointer<IPlatformString>;
using PlatformTimerPtr = SharedPointer<IPlatformTimer>;
using PlatformResourceInputStreamPtr = std::unique_ptr<IPlatformResourceInputStream>;
using PlatformFactoryPtr = std::unique_ptr<IPlatformFactory>;
using PlatformGradientPtr = std::unique_ptr<IPlatformGradient>;
using PlatformGraphicsPathPtr = std::unique_ptr<IPlatformGraphicsPath>;
using PlatformGraphicsPathFactoryPtr = std::shared_ptr<IPlatformGraphicsPathFactory>;
using PlatformFileSelectorPtr = std::shared_ptr<IPlatformFileSelector>;
using PlatformGraphicsDevicePtr = std::shared_ptr<IPlatformGraphicsDevice>;
using PlatformGraphicsDeviceContextPtr = std::shared_ptr<IPlatformGraphicsDeviceContext>;

} // VSTGUI
