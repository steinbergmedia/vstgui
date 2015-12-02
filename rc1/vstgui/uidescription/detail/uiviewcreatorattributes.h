//
//  uiviewcreatorattributes.h
//  vstgui
//
//  Created by Arne Scheffler on 30/10/15.
//
//

#ifndef __uiviewcreatorattributes__
#define __uiviewcreatorattributes__

#include "../iuidescription.h"
#include <cstring>

namespace VSTGUI {
namespace UIViewCreator {

//-----------------------------------------------------------------------------
// view names
//-----------------------------------------------------------------------------
static const IdStringPtr kCView = "CView";
static const IdStringPtr kCViewContainer = "CViewContainer";
static const IdStringPtr kCLayeredViewContainer = "CLayeredViewContainer";
static const IdStringPtr kCRowColumnView = "CRowColumnView";
static const IdStringPtr kCScrollView = "CScrollView";
static const IdStringPtr kUIViewSwitchContainer = "UIViewSwitchContainer";
static const IdStringPtr kCSplitView = "CSplitView";
static const IdStringPtr kCShadowViewContainer = "CShadowViewContainer";

static const IdStringPtr kCControl = "CControl";
static const IdStringPtr kCOnOffButton = "COnOffButton";
static const IdStringPtr kCCheckBox = "CCheckBox";
static const IdStringPtr kCParamDisplay = "CParamDisplay";
static const IdStringPtr kCXYPad = "CXYPad";
static const IdStringPtr kCOptionMenu = "COptionMenu";
static const IdStringPtr kCTextLabel = "CTextLabel";
static const IdStringPtr kCTextEdit = "CTextEdit";
static const IdStringPtr kCTextButton = "CTextButton";
static const IdStringPtr kCSegmentButton = "CSegmentButton";
static const IdStringPtr kCKnob = "CKnob";
static const IdStringPtr kCAnimKnob = "CAnimKnob";
static const IdStringPtr kCVerticalSwitch = "CVerticalSwitch";
static const IdStringPtr kCHorizontalSwitch = "CHorizontalSwitch";
static const IdStringPtr kCRockerSwitch = "CRockerSwitch";
static const IdStringPtr kCMovieBitmap = "CMovieBitmap";
static const IdStringPtr kCMovieButton = "CMovieButton";
static const IdStringPtr kCKickButton = "CKickButton";
static const IdStringPtr kCSlider = "CSlider";
static const IdStringPtr kCVuMeter = "CVuMeter";
static const IdStringPtr kCAnimationSplashScreen = "CAnimationSplashScreen";
static const IdStringPtr kCGradientView = "CGradientView";

//-----------------------------------------------------------------------------
// attributes used in more than one view creator
//-----------------------------------------------------------------------------
static const std::string kAttrClass = "class";
static const std::string kAttrTitle = "title";
static const std::string kAttrFont = "font";
static const std::string kAttrFontColor = "font-color";
static const std::string kAttrFrameColor = "frame-color";
static const std::string kAttrTextAlignment = "text-alignment";
static const std::string kAttrRoundRectRadius = "round-rect-radius";
static const std::string kAttrFrameWidth = "frame-width";
static const std::string kAttrGradientStartColor = "gradient-start-color";
static const std::string kAttrGradientEndColor = "gradient-end-color";
static const std::string kAttrZoomFactor = "zoom-factor";
static const std::string kAttrHandleBitmap = "handle-bitmap";
static const std::string kAttrOrientation = "orientation";
static const std::string kAttrAnimationTime = "animation-time";
static const std::string kAttrGradient = "gradient";

//-----------------------------------------------------------------------------
// CViewCreator attributes
//-----------------------------------------------------------------------------
static const std::string kAttrOrigin = "origin";
static const std::string kAttrSize = "size";
static const std::string kAttrTransparent = "transparent";
static const std::string kAttrMouseEnabled = "mouse-enabled";
static const std::string kAttrBitmap = "bitmap";
static const std::string kAttrDisabledBitmap = "disabled-bitmap";
static const std::string kAttrAutosize = "autosize";
static const std::string kAttrTooltip = "tooltip";
static const std::string kAttrCustomViewName = IUIDescription::kCustomViewName;
static const std::string kAttrSubController = "sub-controller";
static const std::string kAttrOpacity = "opacity";

//-----------------------------------------------------------------------------
// CViewContainerCreator attributes
//-----------------------------------------------------------------------------
static const std::string kAttrBackgroundColor = "background-color";
static const std::string kAttrBackgroundColorDrawStyle = "background-color-draw-style";

//-----------------------------------------------------------------------------
// CLayeredViewContainerCreator attributes
//-----------------------------------------------------------------------------
static const std::string kAttrZIndex = "z-index";

//-----------------------------------------------------------------------------
// CRowColumnViewCreator attributes
//-----------------------------------------------------------------------------
static const std::string kAttrRowStyle = "row-style";
static const std::string kAttrSpacing = "spacing";
static const std::string kAttrMargin = "margin";
static const std::string kAttrAnimateViewResizing = "animate-view-resizing";
static const std::string kAttrEqualSizeLayout = "equal-size-layout";
static const std::string kAttrViewResizeAnimationTime = "view-resize-animation-time";

//-----------------------------------------------------------------------------
// CScrollViewCreator attributes
//-----------------------------------------------------------------------------
static const std::string kAttrContainerSize = "container-size";
static const std::string kAttrHorizontalScrollbar = "horizontal-scrollbar";
static const std::string kAttrVerticalScrollbar = "vertical-scrollbar";
static const std::string kAttrAutoDragScrolling = "auto-drag-scrolling";
static const std::string kAttrBordered = "bordered";
static const std::string kAttrOverlayScrollbars = "overlay-scrollbars";
static const std::string kAttrFollowFocusView = "follow-focus-view";
static const std::string kAttrAutoHideScrollbars = "auto-hide-scrollbars";
static const std::string kAttrScrollbarBackgroundColor = "scrollbar-background-color";
static const std::string kAttrScrollbarFrameColor = "scrollbar-frame-color";
static const std::string kAttrScrollbarScrollerColor = "scrollbar-scroller-color";
static const std::string kAttrScrollbarWidth = "scrollbar-width";

//-----------------------------------------------------------------------------
// CControlCreator attributes
//-----------------------------------------------------------------------------
static const std::string kAttrControlTag = "control-tag";
static const std::string kAttrDefaultValue = "default-value";
static const std::string kAttrMinValue = "min-value";
static const std::string kAttrMaxValue = "max-value";
static const std::string kAttrWheelIncValue = "wheel-inc-value";
static const std::string kAttrBackgroundOffset = "background-offset";

//-----------------------------------------------------------------------------
// CCheckBoxCreator attributes
//-----------------------------------------------------------------------------
static const std::string kAttrBoxframeColor = "boxframe-color";
static const std::string kAttrBoxfillColor = "boxfill-color";
static const std::string kAttrCheckmarkColor = "checkmark-color";
static const std::string kAttrDrawCrossbox = "draw-crossbox";
static const std::string kAttrAutosizeToFit = "autosize-to-fit";

//-----------------------------------------------------------------------------
// CParamDisplayCreator attributes
//-----------------------------------------------------------------------------
static const std::string kAttrBackColor = "back-color";
static const std::string kAttrShadowColor = "shadow-color";
static const std::string kAttrFontAntialias = "font-antialias";
static const std::string kAttrStyle3DIn = "style-3D-in";
static const std::string kAttrStyle3DOut = "style-3D-out";
static const std::string kAttrStyleNoFrame = "style-no-frame";
static const std::string kAttrStyleNoText = "style-no-text";
static const std::string kAttrStyleNoDraw = "style-no-draw";
static const std::string kAttrStyleShadowText = "style-shadow-text";
static const std::string kAttrStyleRoundRect = "style-round-rect";
static const std::string kAttrTextInset = "text-inset";
static const std::string kAttrValuePrecision = "value-precision";
static const std::string kAttrTextRotation = "text-rotation";

//-----------------------------------------------------------------------------
// COptionMenuCreator attributes
//-----------------------------------------------------------------------------
static const std::string kAttrMenuPopupStyle = "menu-popup-style";
static const std::string kAttrMenuCheckStyle = "menu-check-style";

//-----------------------------------------------------------------------------
// CTextLabelCreator attributes
//-----------------------------------------------------------------------------
static const std::string kAttrTruncateMode = "truncate-mode";

//-----------------------------------------------------------------------------
// CTextEditCreator attributes
//-----------------------------------------------------------------------------
static const std::string kAttrImmediateTextChange = "immediate-text-change";
static const std::string kAttrStyleDoubleClick = "style-doubleclick";

//-----------------------------------------------------------------------------
// CTextButtonCreator attributes
//-----------------------------------------------------------------------------
static const std::string kAttrTextColor = "text-color";
static const std::string kAttrTextColorHighlighted = "text-color-highlighted";
static const std::string kAttrGradientStartColorHighlighted = "gradient-start-color-highlighted";
static const std::string kAttrGradientEndColorHighlighted = "gradient-end-color-highlighted";
static const std::string kAttrFrameColorHighlighted = "frame-color-highlighted";
static const std::string kAttrRoundRadius = "round-radius";
static const std::string kAttrKickStyle = "kick-style";
static const std::string kAttrIcon = "icon";
static const std::string kAttrIconHighlighted = "icon-highlighted";
static const std::string kAttrIconPosition = "icon-position";
static const std::string kAttrIconTextMargin = "icon-text-margin";
static const std::string kAttrGradientHighlighted = "gradient-highlighted";

//-----------------------------------------------------------------------------
// CSegmentButtonCreator attributes
//-----------------------------------------------------------------------------
static const std::string kAttrStyle = "style";
static const std::string kAttrSegmentNames = "segment-names";

//-----------------------------------------------------------------------------
// CKnobCreator attributes
//-----------------------------------------------------------------------------
static const std::string kAttrAngleStart = "angle-start";
static const std::string kAttrAngleRange = "angle-range";
static const std::string kAttrValueInset = "value-inset";
static const std::string kAttrCoronaInset = "corona-inset";
static const std::string kAttrCoronaColor = "corona-color";
static const std::string kAttrCoronaDrawing = "corona-drawing";
static const std::string kAttrCoronaOutline = "corona-outline";
static const std::string kAttrCoronaInverted = "corona-inverted";
static const std::string kAttrCoronaFromCenter = "corona-from-center";
static const std::string kAttrCoronaDashDot = "corona-dash-dot";
static const std::string kAttrHandleColor = "handle-color";
static const std::string kAttrHandleShadowColor = "handle-shadow-color";
static const std::string kAttrHandleLineWidth = "handle-line-width";
static const std::string kAttrCircleDrawing = "circle-drawing";

//-----------------------------------------------------------------------------
// IMultiBitmapControlCreator attributes
//-----------------------------------------------------------------------------
static const std::string kAttrHeightOfOneImage = "height-of-one-image";
static const std::string kAttrSubPixmaps = "sub-pixmaps";

//-----------------------------------------------------------------------------
// CSliderCreator attributes
//-----------------------------------------------------------------------------
static const std::string kAttrTransparentHandle = "transparent-handle";
static const std::string kAttrMode = "mode";
static const std::string kAttrHandleOffset = "handle-offset";
static const std::string kAttrBitmapOffset = "bitmap-offset";
static const std::string kAttrReverseOrientation = "reverse-orientation";
static const std::string kAttrDrawFrame = "draw-frame";
static const std::string kAttrDrawBack = "draw-back";
static const std::string kAttrDrawValue = "draw-value";
static const std::string kAttrDrawValueInverted = "draw-value-inverted";
static const std::string kAttrDrawValueFromCenter = "draw-value-from-center";
static const std::string kAttrDrawFrameColor = "draw-frame-color";
static const std::string kAttrDrawBackColor = "draw-back-color";
static const std::string kAttrDrawValueColor = "draw-value-color";

//-----------------------------------------------------------------------------
// CVuMeterCreator attributes
//-----------------------------------------------------------------------------
static const std::string kAttrOffBitmap = "off-bitmap";
static const std::string kAttrNumLed = "num-led";
static const std::string kAttrDecreaseStepValue = "decrease-step-value";

//-----------------------------------------------------------------------------
// CAnimationSplashScreenCreator attributes
//-----------------------------------------------------------------------------
static const std::string kAttrSplashBitmap = "splash-bitmap";
static const std::string kAttrSplashOrigin = "splash-origin";
static const std::string kAttrSplashSize = "splash-size";
static const std::string kAttrAnimationIndex = "animation-index";

//-----------------------------------------------------------------------------
// UIViewSwitchContainerCreator attributes
//-----------------------------------------------------------------------------
static const std::string kAttrTemplateNames = "template-names";
static const std::string kAttrTemplateSwitchControl = "template-switch-control";
static const std::string kAttrAnimationStyle = "animation-style";

//-----------------------------------------------------------------------------
// CSplitViewCreator attributes
//-----------------------------------------------------------------------------
static const std::string kAttrSeparatorWidth = "separator-width";
static const std::string kAttrResizeMethod = "resize-method";

//-----------------------------------------------------------------------------
// CShadowViewContainerCreator attributes
//-----------------------------------------------------------------------------
static const std::string kAttrShadowIntensity = "shadow-intensity";
static const std::string kAttrShadowBlurSize = "shadow-blur-size";
static const std::string kAttrShadowOffset = "shadow-offset";

//-----------------------------------------------------------------------------
// CGradientViewCreator attributes
//-----------------------------------------------------------------------------
static const std::string kAttrGradientAngle = "gradient-angle";
static const std::string kAttrGradientStyle = "gradient-style";
static const std::string kAttrGradientStartColorOffset = "gradient-start-color-offset";
static const std::string kAttrGradientEndColorOffset = "gradient-end-color-offset";
static const std::string kAttrDrawAntialiased = "draw-antialiased";
static const std::string kAttrRadialCenter = "radial-center";
static const std::string kAttrRadialRadius = "radial-radius";


} // UIViewCreator
} // VSTGUI

#endif /* __uiviewcreatorattributes__ */
