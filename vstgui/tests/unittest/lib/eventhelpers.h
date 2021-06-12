// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../../lib/events.h"

//------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------
template <typename EventType>
inline uint32_t dispatchMouseEvent (CView* view, CPoint pos, MouseEventButtonState buttons = {},
                                    Modifiers mods = {})
{
	EventType event;
	event.mousePosition = pos;
	event.buttonState = buttons;
	event.modifiers = mods;
	view->dispatchEvent (event);
	return event.consumed.data;
}

//------------------------------------------------------------------------
inline uint32_t dispatchMouseCancelEvent (CView* view)
{
	MouseCancelEvent event;
	view->dispatchEvent (event);
	return event.consumed.data;
}

//------------------------------------------------------------------------
inline uint32_t dispatchMouseWheelEvent (CView* view, CPoint pos, double deltaX, double deltaY,
                                         Modifiers mods = {})
{
	MouseWheelEvent event;
	event.mousePosition = pos;
	event.deltaX = deltaX;
	event.deltaY = deltaY;
	event.modifiers = mods;
	view->dispatchEvent (event);
	return event.consumed.data;
}

//------------------------------------------------------------------------
} // VSTGUI
