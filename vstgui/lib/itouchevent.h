// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

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
	using TouchPair = std::pair<int32_t, ITouchEvent::Touch>;
	using TouchMap = std::map<int32_t, Touch>;
	
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
			it->second.target = nullptr;
			return true;
		}
		return false;
	}

	virtual double getTimeStamp () const = 0;
protected:
	ITouchEvent () {}
	virtual ~ITouchEvent () noexcept = default;

	TouchMap touches;
};

} // VSTGUI

#endif // VSTGUI_TOUCH_EVENT_HANDLING
