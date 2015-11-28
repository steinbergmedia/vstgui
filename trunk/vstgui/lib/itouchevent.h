//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework not only for VST plugins :
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

#ifndef __itouchevent__
#define __itouchevent__

#include "vstguifwd.h"
#include "cpoint.h"

#if VSTGUI_TOUCH_EVENT_HANDLING

#include <map>

namespace VSTGUI {

//-----------------------------------------------------------------------------
//! @brief a touch event
///	@ingroup new_in_4_2
//-----------------------------------------------------------------------------
class ITouchEvent
{
public:
	enum TouchState {
		kBegan,
		kMoved,
		kEnded,
		kNoChange,
		kCanceled,
		kUndefined
	};
	struct Touch {
		double timeStamp;
		TouchState state;
		CPoint location;
		CView* target;
		bool targetIsSingleTouch;
		uint32_t tapCount;
		
		Touch () : timeStamp (0), state (kUndefined), target (0), targetIsSingleTouch (false), tapCount (0) {}
	};
	typedef std::pair<int32_t, ITouchEvent::Touch> TouchPair;
	typedef std::map<int32_t, Touch> TouchMap;
	
	int32_t numTouches () const { return static_cast<int32_t> (touches.size ()); }
	
	TouchMap::const_iterator begin () const { return touches.begin (); }
	TouchMap::const_iterator end () const { return touches.end (); }
	
	const Touch* find (int32_t identifier) const
	{
		TouchMap::const_iterator it = touches.find (identifier);
		if (it != touches.end ())
			return &(it->second);
		return 0;
	}
	
	bool setTouchTarget (int32_t identifier, CView* view, bool targetIsSingleTouch)
	{
		TouchMap::iterator it = touches.find (identifier);
		if (it != touches.end () && it->second.target == 0)
		{
			it->second.target = view;
			it->second.targetIsSingleTouch = targetIsSingleTouch;
			return true;
		}
		return false;
	}
	
	bool unsetTouchTarget (int32_t identifier, CView* view)
	{
		TouchMap::iterator it = touches.find (identifier);
		if (it != touches.end () && it->second.target == view)
		{
			it->second.target = 0;
			return true;
		}
		return false;
	}

	virtual double getTimeStamp () const = 0;
protected:
	ITouchEvent () {}
	virtual ~ITouchEvent () {}

	TouchMap touches;
};

} // namespace

#endif // VSTGUI_TOUCH_EVENT_HANDLING

#endif // __itouchevent__
