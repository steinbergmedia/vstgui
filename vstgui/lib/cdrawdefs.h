//
//  cdrawdefs.h
//  vstgui
//
//  Created by Arne Scheffler on 06/09/14.
//
//

#ifndef __cdrawdefs__
#define __cdrawdefs__

#include "vstguifwd.h"

namespace VSTGUI {

//-----------
// @brief Draw Mode Flags
//-----------
enum CDrawModeFlags
{
	kAliasing = 0,					///< aliased drawing
	kAntiAliasing = 1,				///< antialised drawing

#if VSTGUI_ENABLE_DEPRECATED_METHODS
	kCopyMode = kAliasing,			///< \deprecated use kAliasing
	kAntialias = kAntiAliasing,		///< \deprecated use kAntiAliasing
#endif

	kNonIntegralMode = 0xF0000000		///< do not round coordinates to pixel aligned values
};

//-----------
// @brief Draw Mode
//-----------
struct CDrawMode
{
public:
	CDrawMode (uint32_t mode = kAliasing) : mode (mode) {}
	CDrawMode (const CDrawMode& m) : mode (m.mode) {}

	uint32_t modeIgnoringIntegralMode () const { return (mode & ~kNonIntegralMode); }

	bool integralMode () const { return mode & kNonIntegralMode ? false : true; }

	CDrawMode& operator= (uint32_t m) { mode = m; return *this; }

	uint32_t operator() () const { return mode; }
	bool operator== (const CDrawMode& m) const { return modeIgnoringIntegralMode () == m.modeIgnoringIntegralMode (); }
	bool operator!= (const CDrawMode& m) const { return modeIgnoringIntegralMode () != m.modeIgnoringIntegralMode (); }
private:
	uint32_t mode;
};

//----------------------------
// @brief Text Alignment (Horizontal)
//----------------------------
enum CHoriTxtAlign
{
	kLeftText = 0,
	kCenterText,
	kRightText
};

//----------------------------
// @brief Draw Style
//----------------------------
enum CDrawStyle
{
	kDrawStroked = 0,
	kDrawFilled,
	kDrawFilledAndStroked
};

}

#endif // __cdrawdefs__
