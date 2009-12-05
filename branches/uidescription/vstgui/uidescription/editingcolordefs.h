#ifndef __editingcolordefs__
#define __editingcolordefs__

#if VSTGUI_LIVE_EDITING

#include "../lib/ccolor.h"

namespace VSTGUI {

#if MAC
static CColor uidPanelBackgroundColor (kTransparentCColor);
#elif WINDOWS
static CColor uidPanelBackgroundColor (kBlackCColor);
#endif

static CColor uidFocusColor (100, 100, 255, 200);

static CColor uidHilightColor (255, 255, 255, 150);
static CColor uidSelectionColor (255, 0, 0, 255);
static CColor uidSelectionHandleColor (255, 0, 0, 150);

static CColor uidCrossLinesBackground (255, 255, 255, 200);
static CColor uidCrossLinesForeground (0, 0, 0, 255);

static CColor uidDataBrowserLineColor (255, 255, 255, 30);
static CColor uidDataBrowserSelectionColor (255, 255, 255, 30);

static CColor uidViewAttributeValueBackgroundColor (255, 255, 255, 150);
static CColor uidViewAttributeDifferentValuesBackgroundColor (150, 150, 255, 150);
static CColor uidViewAttributeValueFrameColor (0, 0, 0, 180);

} // namespace VSTGUI

#endif // VSTGUI_LIVE_EDITING

#endif // __editingcolordefs__