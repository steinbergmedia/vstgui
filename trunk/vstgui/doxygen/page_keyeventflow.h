/**
@page key_event_flow Keyboard Event Flow

@section short_story Short Story

Keyboard events are dispatched from CFrame in this order :

- IKeyboardHook
- focus view
- parents of focus view
- modal view

@section long_story Long Story

If a keyboard event is coming to CFrame::onKeyDown or CFrame::onKeyUp, CFrame will first sent the event to the keyboard hook if 
it is set. If the keyboard hook has not handled the event, the next candidate is the focus view. If there is a focus view and 
the focus view does not handle the event, the event is dispatched to the parent of the focus view. If the parent also does not 
handle the event the event is propagated to the parent of the parent and so on until the parent is the frame.
If the event is still not handled the event will be passed on to the modal view if it exists.

*/
