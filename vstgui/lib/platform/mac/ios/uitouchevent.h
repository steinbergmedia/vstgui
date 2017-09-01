// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __uitouchevent__
#define __uitouchevent__

#include "../../../itouchevent.h"

#if VSTGUI_TOUCH_EVENT_HANDLING

#ifdef __OBJC__
@class UITouch;
#else
struct UITouch;
#endif

namespace VSTGUI {

class UITouchEvent : public ITouchEvent
{
public:
	typedef std::map<UITouch*, int32_t> NativeTouches;

	int32_t touchCounter;
	double currentTime;
	NativeTouches nativeTouches;

	UITouchEvent () : touchCounter (0) {}
	
	TouchMap& getTouchMap () { return touches; }
	double getTimeStamp () const override { return currentTime; }
};

} // namespace

#endif // VSTGUI_TOUCH_EVENT_HANDLING

#endif // __uitouchevent__
