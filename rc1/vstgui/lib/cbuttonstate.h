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

#ifndef __cbuttonstate__
#define __cbuttonstate__

#include "vstguibase.h"

namespace VSTGUI {

//----------------------------
// @brief Button Types (+modifiers)
//----------------------------
enum CButton
{
	kLButton			= 1 << 1,	///< left mouse button
	kMButton			= 1 << 2,	///< middle mouse button
	kRButton			= 1 << 3,	///< right mouse button
	kShift				= 1 << 4,	///< shift modifier
	kControl			= 1 << 5,	///< control modifier (Command Key on Mac OS X and Control Key on Windows)
	kAlt				= 1 << 6,	///< alt modifier
	kApple				= 1 << 7,	///< apple modifier (Mac OS X only. Is the Control key)
	kButton4			= 1 << 8,	///< 4th mouse button
	kButton5			= 1 << 9,	///< 5th mouse button
	kDoubleClick		= 1 << 10,	///< mouse button is double click
	kMouseWheelInverted	= 1 << 11	///< system mouse wheel setting is inverted (Only valid for onMouseWheel methods). The distance value is already transformed back to non inverted. But for scroll views we need to know if we need to invert it back.
};

//-----------------------------------------------------------------------------
// CButtonState Declaration
//! @brief Button and Modifier state
//-----------------------------------------------------------------------------
struct CButtonState
{
public:
	CButtonState (int32_t state = 0) : state (state) {}
	CButtonState (const CButtonState& bs) : state (bs.state) {}
	
	int32_t getButtonState () const { return state & (kLButton | kRButton | kMButton | kButton4 | kButton5); }
	int32_t getModifierState () const { return state & (kShift | kAlt | kControl | kApple); }

	/** returns true if only the left button is set. Ignores modifier state */
	bool isLeftButton () const { return getButtonState () == kLButton; }
	/** returns true if only the right button is set. Ignores modifier state */
	bool isRightButton () const { return getButtonState () == kRButton; }

	bool isDoubleClick () const { return (state & kDoubleClick) ? true : false; }

	int32_t operator() () const { return state; }
	CButtonState& operator= (int32_t s) { state = s; return *this; }
	CButtonState& operator&= (int32_t s) { state &= s; return *this; }
	CButtonState& operator|= (int32_t s) { state |= s; return *this; }

	int32_t operator& (const CButtonState& s) const { return state & s.state; }
	int32_t operator| (const CButtonState& s) const { return state | s.state; }
	int32_t operator~ () const { return ~state; }

	bool operator== (const CButtonState& s) const { return state == s.state; }
	bool operator!= (const CButtonState& s) const { return state != s.state; }
protected:
	int32_t state;
};

}

#endif // __cbuttonstate__
