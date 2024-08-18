// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "scriptobject.h"
#include "../../lib/vstguifwd.h"
#include "../../uidescription/uidescriptionfwd.h"
#include "../../lib/events.h"
#include "../../lib/crect.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace ScriptingInternal {

//------------------------------------------------------------------------
inline ScriptObject makeScriptRect (const CRect& rect)
{
	using namespace std::literals;
	ScriptObject obj;
	obj.addChild ("left"sv, rect.left);
	obj.addChild ("top"sv, rect.top);
	obj.addChild ("right"sv, rect.right);
	obj.addChild ("bottom"sv, rect.bottom);
	obj.addFunc ("getWidth"sv, "{ return this.right - this.left; }"sv);
	obj.addFunc ("getHeight"sv, "{ return this.bottom - this.top; }"sv);
	return obj;
}

//------------------------------------------------------------------------
inline ScriptObject makeScriptPoint (const CPoint& point)
{
	using namespace std::literals;
	ScriptObject obj;
	obj.addChild ("x"sv, point.x);
	obj.addChild ("y"sv, point.y);
	return obj;
}

//------------------------------------------------------------------------
inline CPoint fromScriptPoint (TJS::CScriptVar& var)
{
	using namespace std::literals;
	CPoint result {};
	if (auto xVar = var.findChild ("x"sv))
		result.x = xVar->getVar ()->getDouble ();
	else
		throw TJS::CScriptException ("Not a point object, missing 'x' member");
	if (auto yVar = var.findChild ("y"sv))
		result.y = yVar->getVar ()->getDouble ();
	else
		throw TJS::CScriptException ("Not a point object, missing 'y' member");
	return result;
}

//------------------------------------------------------------------------
inline CRect fromScriptRect (TJS::CScriptVar& var)
{
	using namespace std::literals;
	CRect result {};
	auto leftVar = var.findChild ("left"sv);
	auto topVar = var.findChild ("top"sv);
	auto rightVar = var.findChild ("right"sv);
	auto bottomVar = var.findChild ("bottom"sv);
	if (!leftVar || !topVar || !rightVar || !bottomVar)
		throw TJS::CScriptException ("Expecting a rect object here");
	result.left = leftVar->getVar ()->getDouble ();
	result.top = topVar->getVar ()->getDouble ();
	result.right = rightVar->getVar ()->getDouble ();
	result.bottom = bottomVar->getVar ()->getDouble ();
	return result;
}

//------------------------------------------------------------------------
inline ScriptObject makeScriptEvent (const Event& event)
{
	using namespace std::literals;
	ScriptObject obj;
	if (auto modifierEvent = asModifierEvent (event))
	{
		ScriptObject mod;
		if (modifierEvent->modifiers.has (ModifierKey::Shift))
			mod.addChild ("shift"sv, true);
		if (modifierEvent->modifiers.has (ModifierKey::Alt))
			mod.addChild ("alt"sv, true);
		if (modifierEvent->modifiers.has (ModifierKey::Control))
			mod.addChild ("control"sv, true);
		if (modifierEvent->modifiers.has (ModifierKey::Super))
			mod.addChild ("super"sv, true);
		obj.addChild ("modifiers"sv, std::move (mod));
	}
	if (auto mouseEvent = asMousePositionEvent (event))
	{
		obj.addChild ("mousePosition"sv, makeScriptPoint (mouseEvent->mousePosition));
	}
	if (auto mouseEvent = asMouseEvent (event))
	{
		ScriptObject buttons;
		if (mouseEvent->buttonState.has (MouseButton::Left))
			buttons.addChild ("left"sv, true);
		if (mouseEvent->buttonState.has (MouseButton::Right))
			buttons.addChild ("right"sv, true);
		if (mouseEvent->buttonState.has (MouseButton::Middle))
			buttons.addChild ("middle"sv, true);
		obj.addChild ("mouseButtons"sv, std::move (buttons));
	}
	if (event.type == EventType::MouseWheel)
	{
		const auto& wheelEvent = castMouseWheelEvent (event);
		ScriptObject wheel;
		wheel.addChild ("deltaX"sv, wheelEvent.deltaX);
		wheel.addChild ("deltaY"sv, wheelEvent.deltaY);
		if (wheelEvent.flags & MouseWheelEvent::Flags::DirectionInvertedFromDevice)
			wheel.addChild ("directionInvertedFromDevice"sv, true);
		if (wheelEvent.flags & MouseWheelEvent::Flags::PreciseDeltas)
			wheel.addChild ("preciceDelta"sv, true);
		obj.addChild ("mouseWheel"sv, std::move (wheel));
	}
	if (auto keyEvent = asKeyboardEvent (event))
	{
		ScriptObject key;
		key.addChild ("character"sv, static_cast<int> (keyEvent->character));
		key.addChild ("virtual"sv, static_cast<int> (keyEvent->virt));
		key.addChild ("isRepeat"sv, keyEvent->isRepeat);
		obj.addChild ("key"sv, std::move (key));
	}
	obj.addChild ("consume"sv, 0);
	return obj;
}

//------------------------------------------------------------------------
} // ScriptingInternal
} // VSTGUI
