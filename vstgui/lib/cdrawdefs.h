// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __cdrawdefs__
#define __cdrawdefs__

#include "vstguifwd.h"

namespace VSTGUI {

//-----------
// @brief Draw Mode Flags
//-----------
enum CDrawModeFlags : uint32_t
{
	kAliasing = 0,					///< aliased drawing
	kAntiAliasing = 1,				///< antialised drawing
	kNonIntegralMode = 0xF0000000		///< do not round coordinates to pixel aligned values
};

//-----------
// @brief Draw Mode
//-----------
struct CDrawMode
{
public:
	constexpr CDrawMode (uint32_t mode = kAliasing) : mode (mode) {}
	constexpr CDrawMode (const CDrawMode& m) : mode (m.mode) {}

	constexpr uint32_t modeIgnoringIntegralMode () const { return (mode & ~kNonIntegralMode); }

	constexpr bool integralMode () const { return !hasBit (mode, kNonIntegralMode); }
	constexpr bool aliasing () const { return modeIgnoringIntegralMode () == kAliasing; }
	constexpr bool antiAliasing () const { return modeIgnoringIntegralMode () == kAntiAliasing; }

	CDrawMode& operator= (uint32_t m) { mode = m; return *this; }

	constexpr uint32_t operator() () const { return mode; }
	constexpr bool operator== (const CDrawMode& m) const { return modeIgnoringIntegralMode () == m.modeIgnoringIntegralMode (); }
	constexpr bool operator!= (const CDrawMode& m) const { return modeIgnoringIntegralMode () != m.modeIgnoringIntegralMode (); }
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
