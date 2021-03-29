//------------------------------------------------------------------------
// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE
// Flags : clang-format SMTGSequencer

#include "keyboardview.h"
#include "vstgui/lib/cbitmap.h"
#include "vstgui/lib/coffscreencontext.h"
#include "vstgui/uidescription/detail/uiviewcreatorattributes.h"
#include "vstgui/uidescription/iviewcreator.h"
#include "vstgui/uidescription/uiattributes.h"
#include "vstgui/uidescription/uiviewcreator.h"
#include "vstgui/uidescription/uiviewfactory.h"
#include <sstream>

//------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------
KeyboardView::KeyboardView ()
{
	setDrawNoteText (true);
}

//------------------------------------------------------------------------
double KeyboardView::calcYParameter (NoteIndex note, CCoord y) const
{
	if (note == -1)
		return 0.f;
	y -= getViewSize ().top;
	auto r = getNoteRect (note);
	return y / r.getHeight ();
}

//------------------------------------------------------------------------
double KeyboardView::calcXParameter (NoteIndex note, CCoord x) const
{
	if (note == -1)
		return 0.f;
	auto r = getNoteRect (note);
	x -= r.left;
	return x / r.getWidth ();
}

#if VSTGUI_TOUCH_EVENT_HANDLING
//------------------------------------------------------------------------
void KeyboardView::onTouchEvent (ITouchEvent& event)
{
	for (auto touch : event)
	{
		if (touch.second.target != 0 && touch.second.target != this)
			continue;
		switch (touch.second.state)
		{
			case ITouchEvent::kBegan:
			{
				onTouchBegin (touch, event);
				break;
			}
			case ITouchEvent::kMoved:
			{
				if (touch.second.target == this)
				{
					onTouchMove (touch, event);
				}
				break;
			}
			case ITouchEvent::kCanceled:
			case ITouchEvent::kEnded:
			{
				if (touch.second.target == this)
				{
					onTouchEnd (touch, event);
				}
				break;
			}
			case ITouchEvent::kNoChange:
			case ITouchEvent::kUndefined: break;
		}
	}
}

//------------------------------------------------------------------------
void KeyboardView::onTouchBegin (const ITouchEvent::TouchPair& touch, ITouchEvent& event)
{
	CPoint where (touch.second.location);
	frameToLocal (where);
	auto note = pointToNote (where, false);
	if (note >= 0)
	{
		for (auto nt : noteTouches)
		{
			if (nt.second.note == note)
			{
				// currently no multiple touches for the same note
				return;
			}
		}
		// TODO: velocity
		NoteTouch noteTouch (note);
		event.setTouchTarget (touch.first, this, false);
		setKeyPressed (note, true);
		if (delegate)
			noteTouch.noteID = delegate->onNoteOn (note, calcXParameter (note, where.x),
			                                       calcYParameter (note, where.y));
		noteTouches.insert (std::pair<int32_t, NoteTouch> (touch.first, noteTouch));
	}
}

//------------------------------------------------------------------------
void KeyboardView::onTouchMove (const ITouchEvent::TouchPair& touch, ITouchEvent& event)
{
	CPoint where (touch.second.location);
	frameToLocal (where);
	auto note = pointToNote (where, false);
	auto noteTouch = noteTouches.find (touch.first);
	if (noteTouch != noteTouches.end ())
	{
		if (note != noteTouch->second.note)
		{
			auto noteOff = noteTouch->second.note;
			auto noteOffID = noteTouch->second.noteID;
			setKeyPressed (noteTouch->second.note, false);
			if (note >= 0)
			{
				noteTouch->second.note = note;
				setKeyPressed (noteTouch->second.note, true);
				if (delegate)
					noteTouch->second.noteID = delegate->onNoteOn (
					    note, calcXParameter (note, where.x), calcYParameter (note, where.y));
			}
			else
			{
				noteTouches.erase (noteTouch);
			}
			if (delegate)
				delegate->onNoteOff (noteOff, noteOffID);
		}
		else if (note == noteTouch->second.note)
		{
			if (delegate)
			{
				delegate->onNoteModulation (noteTouch->second.noteID,
				                            calcXParameter (note, where.x),
				                            calcYParameter (note, where.y));
			}
		}
	}
}

//------------------------------------------------------------------------
void KeyboardView::onTouchEnd (const ITouchEvent::TouchPair& touch, ITouchEvent& event)
{
	event.unsetTouchTarget (touch.first, this);
	auto noteTouch = noteTouches.find (touch.first);
	if (noteTouch != noteTouches.end ())
	{
		setKeyPressed (noteTouch->second.note, false);
		if (delegate)
			delegate->onNoteOff (noteTouch->second.note, noteTouch->second.noteID);
		noteTouches.erase (noteTouch);
	}
}
#else
//------------------------------------------------------------------------
void KeyboardView::doNoteOff ()
{
	if (pressedNote != -1)
	{
		if (delegate)
			delegate->onNoteOff (pressedNote, noteID);
		else
			setKeyPressed (pressedNote, false);
		noteID = -1;
		pressedNote = -1;
	}
}

//------------------------------------------------------------------------
void KeyboardView::doNoteOn (int16_t note, double yPos, double xPos)
{
	pressedNote = note;
	if (pressedNote != -1)
	{
		if (delegate)
			noteID = delegate->onNoteOn (pressedNote, yPos, xPos);
		else
			setKeyPressed (pressedNote, true);
	}
}

//------------------------------------------------------------------------
CMouseEventResult KeyboardView::onMouseDown (CPoint& where, const CButtonState& buttons)
{
	if (buttons.isLeftButton ())
	{
		auto note = pointToNote (where, false);
		if (note != -1)
		{
			doNoteOn (note, calcXParameter (note, where.x), calcYParameter (note, where.y));
		}
		return kMouseEventHandled;
	}
	return kMouseEventNotHandled;
}

//------------------------------------------------------------------------
CMouseEventResult KeyboardView::onMouseMoved (CPoint& where, const CButtonState& buttons)
{
	if (buttons.isLeftButton ())
	{
		auto note = pointToNote (where, false);
		if (note == pressedNote)
		{
			if (delegate)
			{
				delegate->onNoteModulation (noteID, calcXParameter (note, where.x),
				                            calcYParameter (note, where.y));
			}
		}
		else
		{
			doNoteOff ();
			doNoteOn (note, calcXParameter (note, where.x), calcYParameter (note, where.y));
		}
	}
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
CMouseEventResult KeyboardView::onMouseUp (CPoint& where, const CButtonState& buttons)
{
	if (buttons.isLeftButton ())
	{
		doNoteOff ();
		return kMouseEventHandled;
	}
	return kMouseEventNotHandled;
}

//------------------------------------------------------------------------
CMouseEventResult KeyboardView::onMouseCancel ()
{
	doNoteOff ();
	return kMouseEventHandled;
}

#endif

//------------------------------------------------------------------------
void KeyboardViewRangeSelector::registerKeyRangeChangedListener (
    IKeyboardViewKeyRangeChangedListener* listener)
{
	listeners.add (listener);
}

//------------------------------------------------------------------------
void KeyboardViewRangeSelector::unregisterKeyRangeChangedListener (
    IKeyboardViewKeyRangeChangedListener* listener)
{
	listeners.remove (listener);
}

//------------------------------------------------------------------------
void KeyboardViewRangeSelector::drawRect (CDrawContext* context, const CRect& dirtyRect)
{
	KeyboardViewBase::drawRect (context, dirtyRect);

	auto r1 = getNoteRect (selectionRange.position);
	auto r2 = getNoteRect (selectionRange.position + selectionRange.length);

	r1.offset (-r1.getWidth (), 0);
	r2.offset (r2.getWidth (), 0);
	r1.left = getViewSize ().left;
	r2.right = getViewSize ().right;
	r1.bottom = getViewSize ().bottom;
	r2.bottom = getViewSize ().bottom;

	context->setFillColor (CColor (0, 0, 0, 110));
	if (!r1.isEmpty ())
		context->drawRect (r1, kDrawFilled);
	if (!r2.isEmpty ())
		context->drawRect (r2, kDrawFilled);
}

//------------------------------------------------------------------------
void KeyboardViewRangeSelector::setKeyRange (NoteIndex startNote, NumNotes numKeys)
{
	KeyboardViewBase::setKeyRange (startNote, numKeys);
	if (selectionRange.position < getKeyRangeStart ())
		selectionRange.position = getKeyRangeStart ();
}

//------------------------------------------------------------------------
void KeyboardViewRangeSelector::setSelectionRange (const Range& _range)
{
	if (selectionRange != _range)
	{
		selectionRange = _range;
		invalid ();
		listeners.forEach ([this] (auto& listener) { listener->onKeyRangeChanged (this); });
	}
}

//------------------------------------------------------------------------
void KeyboardViewRangeSelector::setSelectionMinMax (NumNotes minRange, NumNotes maxRange)
{
	rangeMin = minRange;
	rangeMax = maxRange;
}

//------------------------------------------------------------------------
auto KeyboardViewRangeSelector::getNumWhiteKeysSelected () const -> NumNotes
{
	NumNotes whiteKeys = 0;
	for (auto i = selectionRange.position; i <= selectionRange.position + selectionRange.length;
	     ++i)
	{
		if (isWhiteKey (i))
			whiteKeys++;
	}
	return whiteKeys;
}

#if VSTGUI_TOUCH_EVENT_HANDLING
//------------------------------------------------------------------------
void KeyboardViewRangeSelector::onTouchBegin (const ITouchEvent::TouchPair& touch,
                                              ITouchEvent& event)
{
	CPoint where (touch.second.location);
	frameToLocal (where);
	auto note = pointToNote (where, false);
	if (note >= 0)
	{
		if (touchIds[0] == -1)
		{
			if (note <= selectionRange.position ||
			    note > selectionRange.position + selectionRange.length)
			{
				touchIds[0] = touch.first;
				touchStartNote[0] = note;
				touchMode = note <= selectionRange.position ? kChangeRangeFront : kChangeRangeBack;
				event.setTouchTarget (touch.first, this, false);
				selectionRangeOnTouchStart = selectionRange;
			}
			else if (note > selectionRange.position &&
			         note < selectionRange.position + selectionRange.length)
			{
				touchIds[0] = touch.first;
				touchStartNote[0] = note;
				touchMode = kMoveRange;
				event.setTouchTarget (touch.first, this, false);
				selectionRangeOnTouchStart = selectionRange;
			}
		}
		else
		{
			if (touchMode != kMoveRange)
			{
			}
			else if (note > touchStartNote[0])
			{
				touchIds[1] = touch.first;
				touchStartNote[1] = note;
				event.setTouchTarget (touch.first, this, false);
				touchMode = kChangeRangeFront;
			}
		}
	}
}

//------------------------------------------------------------------------
static int32_t bound (int32_t value, int32_t min, int32_t max)
{
	if (value < min)
		value = min;
	else if (value > max)
		value = max;
	return value;
}

//------------------------------------------------------------------------
void KeyboardViewRangeSelector::onTouchMove (const ITouchEvent::TouchPair& touch,
                                             ITouchEvent& event)
{
	CPoint where (touch.second.location);
	frameToLocal (where);
	auto note = pointToNote (where, true);
	if (touchIds[0] == touch.first)
	{
		if (touchMode == kMoveRange || touchMode == kChangeRangeFront)
		{
			if (note >= 0)
			{
				auto move = note - touchStartNote[0];
				auto newRangeStart =
				    bound (selectionRangeOnTouchStart.position + move, getKeyRangeStart (),
				           (getKeyRangeStart () + getNumKeys ()) - selectionRange.length);
				if (!isWhiteKey (newRangeStart))
					newRangeStart++;
				auto length = selectionRange.length;
				if (touchMode == kChangeRangeFront)
				{
					length = selectionRange.length + (selectionRange.position - newRangeStart);
					if (length < rangeMin || length > rangeMax)
						return;
				}
				setSelectionRange (Range (newRangeStart, length));
			}
		}
		else if (touchMode == kChangeRangeBack)
		{
			if (note >= 0)
			{
				auto move = note - touchStartNote[0];
				auto length = selectionRangeOnTouchStart.length + move;
				if (!isWhiteKey (selectionRange.position + length))
					length++;
				if (length < rangeMin || length > rangeMax)
					return;
				setSelectionRange (Range (selectionRange.position, length));
			}
		}
	}
	else
	{
	}
}

//------------------------------------------------------------------------
void KeyboardViewRangeSelector::onTouchEvent (ITouchEvent& event)
{
	for (auto touch : event)
	{
		if (touch.second.target != 0 && touch.second.target != this)
			continue;
		switch (touch.second.state)
		{
			case ITouchEvent::kBegan:
			{
				if (touchIds[0] != -1 && touchIds[1] != -1)
					continue;
				onTouchBegin (touch, event);
				break;
			}
			case ITouchEvent::kCanceled:
			case ITouchEvent::kEnded:
			{
				if (touch.second.target == this)
				{
					if (touchIds[0] == touch.first)
					{
						touchIds[0] = -1;
					}
					else
					{
						touchIds[1] = -1;
					}
					event.unsetTouchTarget (touch.first, this);
				}
				break;
			}
			case ITouchEvent::kMoved:
			{
				if (touch.second.target == this)
				{
					onTouchMove (touch, event);
				}
				break;
			}
			case ITouchEvent::kNoChange:
			case ITouchEvent::kUndefined: break;
		}
	}
}

//------------------------------------------------------------------------
bool KeyboardViewRangeSelector::wantsMultiTouchEvents () const
{
	return true;
}
#else

//------------------------------------------------------------------------
CMouseEventResult KeyboardViewRangeSelector::onMouseDown (CPoint& where,
                                                          const CButtonState& buttons)
{
	if (buttons.isLeftButton ())
	{
		moveStartRange = selectionRange;
		moveStartNote = pointToNote (where, true);
		if (moveStartNote < selectionRange.position ||
		    moveStartNote >= selectionRange.position + selectionRange.length)
		{
			auto middle = selectionRange.length / 2;
			if (moveStartNote < middle)
				moveStartRange.position = 0;
			else
				moveStartRange.position = moveStartNote - middle;
			return onMouseMoved (where, buttons);
		}
		return kMouseEventHandled;
	}
	return kMouseEventNotHandled;
}

//------------------------------------------------------------------------
CMouseEventResult KeyboardViewRangeSelector::onMouseMoved (CPoint& where,
                                                           const CButtonState& buttons)
{
	if (buttons.isLeftButton () && moveStartNote != -1)
	{
		auto note = pointToNote (where, true);
		if (note != -1)
		{
			auto offset = note - moveStartNote;
			Range r = moveStartRange;
			if (static_cast<NoteIndex> (r.position) + offset < 0)
				r.position = 0;
			else if (static_cast<NoteIndex> (r.position + r.length) + offset >= MaxNotes)
				r.position = (MaxNotes - 1) - r.length;
			else
				r.position += offset;
			setSelectionRange (r);
		}
	}
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
CMouseEventResult KeyboardViewRangeSelector::onMouseUp (CPoint& where, const CButtonState& buttons)
{
	if (buttons.isLeftButton ())
		moveStartNote = -1;
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
CMouseEventResult KeyboardViewRangeSelector::onMouseCancel ()
{
	moveStartNote = -1;
	return kMouseEventHandled;
}

#endif

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
KeyboardViewBase::KeyboardViewBase () : CView (CRect (0, 0, 0, 0)), noteNameFont (kSystemFont)
{
}

//------------------------------------------------------------------------
void KeyboardViewBase::setViewSize (const CRect& rect, bool invalid)
{
	CView::setViewSize (rect, invalid);
	noteRectCacheInvalid = true;
}

//------------------------------------------------------------------------
bool KeyboardViewBase::sizeToFit ()
{
	if (noteRectCacheInvalid)
		updateNoteRectCache ();

	auto r = getNoteRect (startNote + numKeys - 1);
	r.setWidth (r.right);
	r.setHeight (getViewSize ().getHeight ());
	r.originize ();

	setViewSize (r);
	setMouseableArea (r);
	return true;
}

//------------------------------------------------------------------------
auto KeyboardViewBase::getNumWhiteKeys () const -> NumNotes
{
	NumNotes whiteKeys = 0;
	for (NoteIndex i = startNote; i <= startNote + numKeys; ++i)
	{
		if (isWhiteKey (i))
			whiteKeys++;
	}
	return whiteKeys;
}

//------------------------------------------------------------------------
void KeyboardViewBase::setKeyRange (NoteIndex _startNote, NumNotes _numKeys)
{
	vstgui_assert (_startNote >= 0 && _numKeys >= 0);
	if (_startNote < 0 || _numKeys < 0)
		return;

	if (static_cast<int32_t> (_numKeys) + static_cast<int32_t> (_startNote) >=
	    static_cast<int32_t> (MaxNotes))
	{
		_numKeys = (MaxNotes - 1) - _startNote;
	}
	startNote = _startNote;
	numKeys = _numKeys;
	noteRectCacheInvalid = true;
	invalid ();
}

//------------------------------------------------------------------------
void KeyboardViewBase::setNoteNameFont (CFontDesc* font)
{
	if (font != noteNameFont)
	{
		noteNameFont = font;
		if (drawNoteText)
			invalid ();
	}
}

//------------------------------------------------------------------------
void KeyboardViewBase::setDrawNoteText (bool state)
{
	if (state != drawNoteText)
	{
		drawNoteText = state;
		invalid ();
	}
}

//------------------------------------------------------------------------
void KeyboardViewBase::setWhiteKeyWidth (CCoord width)
{
	if (whiteKeyWidth != width)
	{
		whiteKeyWidth = width;
		whiteKeyBitmapCache = nullptr;
		noteRectCacheInvalid = true;
		invalid ();
	}
}

//------------------------------------------------------------------------
void KeyboardViewBase::setBlackKeyWidth (CCoord width)
{
	if (blackKeyWidth != width)
	{
		blackKeyWidth = width;
		blackKeyBitmapCache = nullptr;
		noteRectCacheInvalid = true;
		invalid ();
	}
}

//------------------------------------------------------------------------
void KeyboardViewBase::setBlackKeyHeight (CCoord height)
{
	if (blackKeyHeight != height)
	{
		blackKeyHeight = height;
		noteRectCacheInvalid = true;
		invalid ();
	}
}

//------------------------------------------------------------------------
void KeyboardViewBase::setLineWidth (CCoord width)
{
	if (lineWidth != width)
	{
		lineWidth = width;
		invalid ();
	}
}

//------------------------------------------------------------------------
void KeyboardViewBase::setFrameColor (CColor color)
{
	if (frameColor != color)
	{
		frameColor = color;
		invalid ();
	}
}

//------------------------------------------------------------------------
void KeyboardViewBase::setFontColor (CColor color)
{
	if (fontColor != color)
	{
		fontColor = color;
		invalid ();
	}
}

//------------------------------------------------------------------------
void KeyboardViewBase::setWhiteKeyColor (CColor color)
{
	if (whiteKeyColor != color)
	{
		whiteKeyColor = color;
		invalid ();
	}
}

//------------------------------------------------------------------------
void KeyboardViewBase::setWhiteKeyPressedColor (CColor color)
{
	if (whiteKeyPressedColor != color)
	{
		whiteKeyPressedColor = color;
		invalid ();
	}
}

//------------------------------------------------------------------------
void KeyboardViewBase::setBlackKeyColor (CColor color)
{
	if (blackKeyColor != color)
	{
		blackKeyColor = color;
		invalid ();
	}
}

//------------------------------------------------------------------------
void KeyboardViewBase::setBlackKeyPressedColor (CColor color)
{
	if (blackKeyPressedColor != color)
	{
		blackKeyPressedColor = color;
		invalid ();
	}
}

//------------------------------------------------------------------------
void KeyboardViewBase::createBitmapCache ()
{
	auto whiteKeyBitmap = getBitmap (BitmapID::WhiteKeyUnpressed);
	auto blackKeyBitmap = getBitmap (BitmapID::BlackKeyUnpressed);
	if (!whiteKeyBitmap || !blackKeyBitmap)
		return;

#if ((VSTGUI_VERSION_MAJOR == 4 && VSTGUI_VERSION_MINOR > 9) || (VSTGUI_VERSION_MAJOR > 4))
	if (auto offscreen = COffscreenContext::create (CPoint (whiteKeyWidth, getHeight ())))
#else
	if (auto offscreen = COffscreenContext::create (getFrame (), whiteKeyWidth, getHeight ()))
#endif
	{
		offscreen->beginDraw ();
		CRect r (0, 0, whiteKeyWidth, getHeight ());
		r.left -= whiteKeyBitmapInset.left;
		r.right += whiteKeyBitmapInset.right;
		r.top -= whiteKeyBitmapInset.top;
		r.bottom += whiteKeyBitmapInset.bottom;
		whiteKeyBitmap->draw (offscreen, r);
		offscreen->endDraw ();
		whiteKeyBitmapCache = offscreen->getBitmap ();
	}

#if ((VSTGUI_VERSION_MAJOR == 4 && VSTGUI_VERSION_MINOR > 9) || (VSTGUI_VERSION_MAJOR > 4))
	if (auto offscreen = COffscreenContext::create (CPoint (blackKeyWidth, blackKeyHeight)))
#else
	if (auto offscreen = COffscreenContext::create (getFrame (), blackKeyWidth, blackKeyHeight))
#endif
	{
		offscreen->beginDraw ();
		CRect r (0, 0, blackKeyWidth, blackKeyHeight);
		r.left -= blackKeyBitmapInset.left;
		r.right += blackKeyBitmapInset.right;
		r.top -= blackKeyBitmapInset.top;
		r.bottom += blackKeyBitmapInset.bottom;
		blackKeyBitmap->draw (offscreen, r);
		offscreen->endDraw ();
		blackKeyBitmapCache = offscreen->getBitmap ();
	}
}

//------------------------------------------------------------------------
void KeyboardViewBase::drawRect (CDrawContext* context, const CRect& dirtyRect)
{
	if (noteRectCacheInvalid)
		updateNoteRectCache ();

	if (whiteKeyBitmapCache == nullptr || blackKeyBitmapCache == nullptr)
		createBitmapCache ();

	context->setLineWidth (lineWidth == -1 ? context->getHairlineSize () : lineWidth);
	context->setFrameColor (frameColor);
	context->setFontColor (fontColor);
	context->setFont (noteNameFont);
	context->setDrawMode (kAntiAliasing | kNonIntegralMode);

	for (NoteIndex i = startNote; i <= startNote + numKeys; i++)
	{
		if (isWhiteKey (i) == false)
			continue;
		CRect r = getNoteRect (i);
		if (dirtyRect.rectOverlap (r) == false)
			continue;
		drawNote (context, r, i, true);
		if (drawNoteText && i % 12 == 0)
		{
			char text[5];
			snprintf (text, 4, "C%d", (i / 12) - 2);
			r.top = r.bottom - context->getFont ()->getSize () - 10;
			context->drawString (text, r);
		}
	}
	for (NoteIndex i = startNote; i <= startNote + numKeys; i++)
	{
		if (isWhiteKey (i) == true)
			continue;
		CRect r = getNoteRect (i);
		if (dirtyRect.rectOverlap (r) == false)
			continue;
		drawNote (context, r, i, false);
	}
}

//------------------------------------------------------------------------
void KeyboardViewBase::drawNote (CDrawContext* context, CRect& rect, NoteIndex note,
                                 bool isWhite) const
{
	CBitmap* keyBitmap = nullptr;
	CRect bitmapRect (rect);
	if (isWhite)
	{
		bitmapRect.left -= whiteKeyBitmapInset.left;
		bitmapRect.right += whiteKeyBitmapInset.right;
		bitmapRect.top -= whiteKeyBitmapInset.top;
		bitmapRect.bottom += whiteKeyBitmapInset.bottom;
	}
	else
	{
		bitmapRect.left -= blackKeyBitmapInset.left;
		bitmapRect.right += blackKeyBitmapInset.right;
		bitmapRect.top -= blackKeyBitmapInset.top;
		bitmapRect.bottom += blackKeyBitmapInset.bottom;
	}

	if (keyPressed[note])
		keyBitmap = getBitmap (isWhite ? BitmapID::WhiteKeyPressed : BitmapID::BlackKeyPressed);
	else
	{
		if (isWhite)
		{
			if (whiteKeyBitmapCache && whiteKeyBitmapCache->getWidth () == bitmapRect.getWidth () &&
			    whiteKeyBitmapCache->getHeight () == bitmapRect.getHeight ())
				keyBitmap = whiteKeyBitmapCache;
			else
				keyBitmap = getBitmap (BitmapID::WhiteKeyUnpressed);
		}
		else
		{
			if (blackKeyBitmapCache && blackKeyBitmapCache->getWidth () == bitmapRect.getWidth () &&
			    blackKeyBitmapCache->getHeight () == bitmapRect.getHeight ())
				keyBitmap = blackKeyBitmapCache;
			else
				keyBitmap = getBitmap (BitmapID::BlackKeyUnpressed);
		}
	}

	if (keyBitmap)
	{
		keyBitmap->draw (context, bitmapRect);
	}
	else
	{
		if (keyPressed[note])
			context->setFillColor (isWhite ? whiteKeyPressedColor : blackKeyPressedColor);
		else
			context->setFillColor (isWhite ? whiteKeyColor : blackKeyColor);
		context->drawRect (rect, isWhite ? kDrawFilledAndStroked : kDrawFilled);
	}
	if (keyPressed[note] && isWhite)
	{
		NoteIndex otherNote;
		if (note > startNote)
		{
			otherNote = note - 1;
			if (!isWhiteKey (otherNote))
				otherNote--;
			if (keyPressed[otherNote] == false)
			{
				if (auto b = getBitmap (BitmapID::WhiteKeyShadowLeft))
				{
					b->draw (context, bitmapRect);
				}
			}
		}

		if (note < startNote + numKeys)
		{
			otherNote = note + 1;
			if (!isWhiteKey (otherNote))
				otherNote++;
			if (keyPressed[otherNote] == false)
			{
				if (auto b = getBitmap (BitmapID::WhiteKeyShadowRight))
				{
					b->draw (context, bitmapRect);
				}
			}
		}
	}
}

//------------------------------------------------------------------------
CRect KeyboardViewBase::calcNoteRect (NoteIndex note) const
{
	CRect result;
	if (note >= startNote && note <= startNote + numKeys)
	{
		for (NoteIndex i = startNote + 1; i <= note; i++)
		{
			bool isWhite = isWhiteKey (i);
			if (isWhite)
			{
				result.left += whiteKeyWidth;
			}
		}
		if (isWhiteKey (note))
		{
			result.setWidth (whiteKeyWidth);
			result.setHeight (getViewSize ().getHeight ());
		}
		else
		{
			result.left += whiteKeyWidth - blackKeyWidth / 2;
			result.setWidth (blackKeyWidth);
			result.setHeight (blackKeyHeight);
		}
	}
	result.offset (getViewSize ().left, getViewSize ().top);
	return result;
}

//------------------------------------------------------------------------
void KeyboardViewBase::updateNoteRectCache () const
{
	for (NoteIndex i = 0; i < MaxNotes; ++i)
		noteRectCache[i] = calcNoteRect (i);

	CRect r = getNoteRect (startNote + numKeys);
	CCoord space = getViewSize ().right - r.right;
	if (space > 0)
	{
		space = fabs (space / 2.);
		for (NoteIndex i = startNote + 1; i <= startNote + numKeys; ++i)
			noteRectCache[i].offset (space, 0);
		noteRectCache[startNote].right += space;
		noteRectCache[startNote + numKeys].right = getViewSize ().right;
	}
	noteRectCacheInvalid = false;
}

//------------------------------------------------------------------------
auto KeyboardViewBase::pointToNote (const CPoint& p, bool ignoreY) const -> NoteIndex
{
	if (noteRectCacheInvalid)
		updateNoteRectCache ();
	NoteIndex result = 0;
	for (auto r : getNoteRectCache ())
	{
		if (!ignoreY)
		{
			if (r.pointInside (p))
			{
				if (isWhiteKey (result))
				{
					if (getNoteRect (result + 1).pointInside (p))
						return result + 1;
				}
				return result;
			}
		}
		else if (p.x >= r.left && p.x < r.right)
		{
			if (isWhiteKey (result))
			{
				auto r2 = getNoteRect (result + 1);
				if (p.x >= r2.left && p.x < r2.right)
					return result + 1;
			}
			return result;
		}
		result++;
	}
	return -1;
}

//------------------------------------------------------------------------
void KeyboardViewBase::invalidNote (NoteIndex note)
{
	if (noteRectCacheInvalid)
		updateNoteRectCache ();
	invalidRect (getNoteRect (note));
}

//------------------------------------------------------------------------
void KeyboardViewBase::setKeyPressed (NoteIndex note, bool state)
{
	vstgui_assert (note >= 0);
	if (note < 0)
		return;

	if (keyPressed[note] != state)
	{
		keyPressed[note] = state;
		invalidNote (note);
		if (isWhiteKey (note))
		{
			if (note > startNote)
			{
				NoteIndex prevKey = note - 1;
				if (!isWhiteKey (prevKey))
					prevKey--;
				invalidNote (prevKey);
			}
			if (note < startNote + numKeys)
			{
				NoteIndex nextKey = note + 1;
				if (!isWhiteKey (nextKey))
					nextKey++;
				invalidNote (nextKey);
			}
		}
	}
}

//------------------------------------------------------------------------
bool KeyboardViewBase::isWhiteKey (NoteIndex note) const
{
	note = note % 12;
	return note == 0 || note == 2 || note == 4 || note == 5 || note == 7 || note == 9 || note == 11;
}

//------------------------------------------------------------------------
void KeyboardViewBase::setWhiteKeyBitmapInset (const CRect& inset)
{
	whiteKeyBitmapInset = inset;
}

//------------------------------------------------------------------------
void KeyboardViewBase::setBlackKeyBitmapInset (const CRect& inset)
{
	blackKeyBitmapInset = inset;
}

//------------------------------------------------------------------------
void KeyboardViewBase::setBitmap (BitmapID bID, CBitmap* bitmap)
{
	bitmaps[static_cast<size_t> (bID)] = bitmap;
	invalid ();
}

//------------------------------------------------------------------------
CBitmap* KeyboardViewBase::getBitmap (BitmapID bID) const
{
	return bitmaps[static_cast<size_t> (bID)];
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
static const std::string kAttrWhiteKeyPressed = "white-key-pressed";
static const std::string kAttrWhiteKeyUnpressed = "white-key-unpressed";
static const std::string kAttrBlackKeyPressed = "black-key-pressed";
static const std::string kAttrBlackKeyUnpressed = "black-key-unpressed";
static const std::string kAttrWhiteKeyShadowLeft = "white-key-shadow-left";
static const std::string kAttrWhiteKeyShadowRight = "white-key-shadow-right";
static const std::string kAttrWhiteKeyWidth = "white-key-width";
static const std::string kAttrBlackKeyWidth = "black-key-width";
static const std::string kAttrBlackKeyHeight = "black-key-height";
static const std::string kAttrWhiteKeyColor = "white-key-color";
static const std::string kAttrWhiteKeyPressedColor = "white-key-pressed-color";
static const std::string kAttrBlackKeyColor = "black-key-color";
static const std::string kAttrBlackKeyPressedColor = "black-key-pressed-color";
static const std::string kAttrStartNote = "start-note";
static const std::string kAttrNumKeys = "num-keys";
static const std::string kAttrNoteNameFont = "note-name-font";
static const std::string kAttrDrawNoteText = "draw-note-text";

using UIViewCreator::stringToColor;
using UIViewCreator::stringToBitmap;
using UIViewCreator::bitmapToString;
using UIViewCreator::colorToString;

//-----------------------------------------------------------------------------
class KeyboardViewBaseCreator : public ViewCreatorAdapter
{
public:
	using ViewType = KeyboardViewBase;

	IdStringPtr getBaseViewName () const override { return UIViewCreator::kCView; }
	bool getAttributeNames (std::list<std::string>& attributeNames) const override
	{
		attributeNames.push_back (kAttrWhiteKeyPressed);
		attributeNames.push_back (kAttrWhiteKeyUnpressed);
		attributeNames.push_back (kAttrBlackKeyPressed);
		attributeNames.push_back (kAttrBlackKeyUnpressed);
		attributeNames.push_back (kAttrWhiteKeyShadowLeft);
		attributeNames.push_back (kAttrWhiteKeyShadowRight);
		attributeNames.push_back (kAttrWhiteKeyWidth);
		attributeNames.push_back (kAttrBlackKeyWidth);
		attributeNames.push_back (kAttrBlackKeyHeight);
		attributeNames.push_back (UIViewCreator::kAttrFrameColor);
		attributeNames.push_back (UIViewCreator::kAttrFontColor);
		attributeNames.push_back (kAttrWhiteKeyColor);
		attributeNames.push_back (kAttrWhiteKeyPressedColor);
		attributeNames.push_back (kAttrBlackKeyColor);
		attributeNames.push_back (kAttrBlackKeyPressedColor);
		attributeNames.push_back (UIViewCreator::kAttrFrameWidth);
		attributeNames.push_back (kAttrStartNote);
		attributeNames.push_back (kAttrNumKeys);
		attributeNames.push_back (kAttrDrawNoteText);
		attributeNames.push_back (kAttrNoteNameFont);
		return true;
	}
	AttrType getAttributeType (const std::string& attributeName) const override
	{
		auto bitmapID = attrNameToBitmapID (attributeName);
		if (bitmapID != ViewType::BitmapID::NumBitmaps)
			return kBitmapType;
		if (attributeName == kAttrWhiteKeyWidth)
			return kFloatType;
		if (attributeName == kAttrBlackKeyWidth)
			return kFloatType;
		if (attributeName == kAttrBlackKeyHeight)
			return kFloatType;
		if (attributeName == kAttrStartNote)
			return kIntegerType;
		if (attributeName == kAttrNumKeys)
			return kIntegerType;
		if (attributeName == kAttrDrawNoteText)
			return kBooleanType;
		if (attributeName == kAttrNoteNameFont)
			return kFontType;
		if (attributeName == UIViewCreator::kAttrFrameWidth)
			return kFloatType;
		if (attributeName == UIViewCreator::kAttrFrameColor)
			return kColorType;
		if (attributeName == UIViewCreator::kAttrFontColor)
			return kColorType;
		if (attributeName == kAttrWhiteKeyColor)
			return kColorType;
		if (attributeName == kAttrWhiteKeyPressedColor)
			return kColorType;
		if (attributeName == kAttrBlackKeyColor)
			return kColorType;
		if (attributeName == kAttrBlackKeyPressedColor)
			return kColorType;
		return kUnknownType;
	}
	bool getAttributeValue (CView* view, const std::string& attributeName, std::string& stringValue,
	                        const IUIDescription* desc) const override
	{
		auto kv = dynamic_cast<ViewType*> (view);
		if (!kv)
			return false;
		auto bitmapID = attrNameToBitmapID (attributeName);
		if (bitmapID != ViewType::BitmapID::NumBitmaps)
		{
			stringValue = "";
			if (auto bitmap = kv->getBitmap (bitmapID))
				bitmapToString (bitmap, stringValue, desc);
			return true;
		}
		if (attributeName == UIViewCreator::kAttrFrameColor)
		{
			colorToString (kv->getFrameColor (), stringValue, desc);
			return true;
		}
		if (attributeName == UIViewCreator::kAttrFontColor)
		{
			colorToString (kv->getFontColor (), stringValue, desc);
			return true;
		}
		if (attributeName == kAttrWhiteKeyColor)
		{
			colorToString (kv->getWhiteKeyColor (), stringValue, desc);
			return true;
		}
		if (attributeName == kAttrWhiteKeyPressedColor)
		{
			colorToString (kv->getWhiteKeyPressedColor (), stringValue, desc);
			return true;
		}
		if (attributeName == kAttrBlackKeyColor)
		{
			colorToString (kv->getBlackKeyColor (), stringValue, desc);
			return true;
		}
		if (attributeName == kAttrBlackKeyPressedColor)
		{
			colorToString (kv->getBlackKeyPressedColor (), stringValue, desc);
			return true;
		}
		if (attributeName == kAttrWhiteKeyWidth)
		{
			stringValue = numberToString (kv->getWhiteKeyWidth ());
			return true;
		}
		if (attributeName == kAttrBlackKeyWidth)
		{
			stringValue = numberToString (kv->getBlackKeyWidth ());
			return true;
		}
		if (attributeName == kAttrBlackKeyHeight)
		{
			stringValue = numberToString (kv->getBlackKeyHeight ());
			return true;
		}
		if (attributeName == kAttrStartNote)
		{
			stringValue = numberToString<int32_t> (kv->getKeyRangeStart ());
			return true;
		}
		if (attributeName == kAttrNumKeys)
		{
			stringValue = numberToString<int32_t> (kv->getNumKeys ());
			return true;
		}
		if (attributeName == UIViewCreator::kAttrFrameWidth)
		{
			stringValue = numberToString (kv->getLineWidth ());
			return true;
		}
		if (attributeName == kAttrDrawNoteText)
		{
			stringValue = kv->getDrawNoteText () ? "true" : "false";
			return true;
		}
		if (attributeName == kAttrNoteNameFont)
		{
			UTF8StringPtr fontName = desc->lookupFontName (kv->getNoteNameFont ());
			if (fontName)
			{
				stringValue = fontName;
				return true;
			}
			return false;
		}
		return false;
	}
	bool apply (CView* view, const UIAttributes& attributes,
	            const IUIDescription* desc) const override
	{
		auto kv = dynamic_cast<ViewType*> (view);
		if (!kv)
			return false;
		CBitmap* bitmap;
		if (stringToBitmap (attributes.getAttributeValue (kAttrWhiteKeyPressed), bitmap, desc))
			kv->setBitmap (ViewType::BitmapID::WhiteKeyPressed, bitmap);
		if (stringToBitmap (attributes.getAttributeValue (kAttrWhiteKeyUnpressed), bitmap, desc))
			kv->setBitmap (ViewType::BitmapID::WhiteKeyUnpressed, bitmap);
		if (stringToBitmap (attributes.getAttributeValue (kAttrBlackKeyPressed), bitmap, desc))
			kv->setBitmap (ViewType::BitmapID::BlackKeyPressed, bitmap);
		if (stringToBitmap (attributes.getAttributeValue (kAttrBlackKeyUnpressed), bitmap, desc))
			kv->setBitmap (ViewType::BitmapID::BlackKeyUnpressed, bitmap);
		if (stringToBitmap (attributes.getAttributeValue (kAttrWhiteKeyShadowLeft), bitmap, desc))
			kv->setBitmap (ViewType::BitmapID::WhiteKeyShadowLeft, bitmap);
		if (stringToBitmap (attributes.getAttributeValue (kAttrWhiteKeyShadowRight), bitmap, desc))
			kv->setBitmap (ViewType::BitmapID::WhiteKeyShadowRight, bitmap);

		CColor color;
		if (stringToColor (attributes.getAttributeValue (UIViewCreator::kAttrFrameColor), color,
		                   desc))
			kv->setFrameColor (color);
		if (stringToColor (attributes.getAttributeValue (UIViewCreator::kAttrFontColor), color,
		                   desc))
			kv->setFontColor (color);
		if (stringToColor (attributes.getAttributeValue (kAttrWhiteKeyColor), color, desc))
			kv->setWhiteKeyColor (color);
		if (stringToColor (attributes.getAttributeValue (kAttrWhiteKeyPressedColor), color, desc))
			kv->setWhiteKeyPressedColor (color);
		if (stringToColor (attributes.getAttributeValue (kAttrBlackKeyColor), color, desc))
			kv->setBlackKeyColor (color);
		if (stringToColor (attributes.getAttributeValue (kAttrBlackKeyPressedColor), color, desc))
			kv->setBlackKeyPressedColor (color);

		CCoord c;
		if (attributes.getDoubleAttribute (kAttrWhiteKeyWidth, c))
			kv->setWhiteKeyWidth (c);
		if (attributes.getDoubleAttribute (kAttrBlackKeyWidth, c))
			kv->setBlackKeyWidth (c);
		if (attributes.getDoubleAttribute (kAttrBlackKeyHeight, c))
			kv->setBlackKeyHeight (c);
		if (attributes.getDoubleAttribute (UIViewCreator::kAttrFrameWidth, c))
			kv->setLineWidth (c);
		auto startNote = static_cast<int32_t> (kv->getKeyRangeStart ());
		auto numKeys = static_cast<int32_t> (kv->getNumKeys ());
		attributes.getIntegerAttribute (kAttrStartNote, startNote);
		attributes.getIntegerAttribute (kAttrNumKeys, numKeys);
		kv->setKeyRange (startNote, numKeys);
		bool b;
		if (attributes.getBooleanAttribute (kAttrDrawNoteText, b))
			kv->setDrawNoteText (b);
		if (auto fontName = attributes.getAttributeValue (kAttrNoteNameFont))
			kv->setNoteNameFont (desc->getFont (fontName->data ()));

		return true;
	}

	bool getAttributeValueRange (const std::string& attributeName, double& minValue,
	                             double& maxValue) const override
	{
		if (attributeName == kAttrNumKeys)
		{
			minValue = 12;
			maxValue = KeyboardViewBase::MaxNotes - 1;
			return true;
		}
		return false;
	}

//-----------------------------------------------------------------------------
	// TODO: this is a clone see uiviewcreator.cpp
	template <typename T>
	static std::string numberToString (T value)
	{
		std::stringstream str;
		str << value;
		return str.str ();
	}

	static ViewType::BitmapID attrNameToBitmapID (const std::string& attributeName)
	{
		if (attributeName == kAttrWhiteKeyPressed)
			return ViewType::BitmapID::WhiteKeyPressed;
		if (attributeName == kAttrWhiteKeyUnpressed)
			return ViewType::BitmapID::WhiteKeyUnpressed;
		if (attributeName == kAttrBlackKeyPressed)
			return ViewType::BitmapID::BlackKeyPressed;
		if (attributeName == kAttrBlackKeyUnpressed)
			return ViewType::BitmapID::BlackKeyUnpressed;
		if (attributeName == kAttrWhiteKeyShadowLeft)
			return ViewType::BitmapID::WhiteKeyShadowLeft;
		if (attributeName == kAttrWhiteKeyShadowRight)
			return ViewType::BitmapID::WhiteKeyShadowRight;
		return ViewType::BitmapID::NumBitmaps;
	}
};

//------------------------------------------------------------------------
class KeyboardViewCreator : public KeyboardViewBaseCreator
{
public:
	KeyboardViewCreator () { UIViewFactory::registerViewCreator (*this); }
	IdStringPtr getViewName () const override { return "KeyboardView"; }
	UTF8StringPtr getDisplayName () const override { return "Keyboard"; }
	CView* create (const UIAttributes& attributes, const IUIDescription* description) const override
	{
		return new KeyboardView ();
	}
};
KeyboardViewCreator __gKeyboardViewCreator;

static const std::string kAttrSelectionRangeMin = "sel-range-min";
static const std::string kAttrSelectionRangeMax = "sel-range-max";
//------------------------------------------------------------------------
class KeyboardViewRangeSelectorCreator : public KeyboardViewBaseCreator
{
public:
	KeyboardViewRangeSelectorCreator () { UIViewFactory::registerViewCreator (*this); }
	IdStringPtr getViewName () const override { return "KeyboardViewRangeSelector"; }
	UTF8StringPtr getDisplayName () const override { return "Keyboard Range Selector"; }
	CView* create (const UIAttributes& attributes, const IUIDescription* description) const override
	{
		return new KeyboardViewRangeSelector ();
	}
	bool getAttributeNames (std::list<std::string>& attributeNames) const override
	{
		attributeNames.push_back (kAttrSelectionRangeMin);
		attributeNames.push_back (kAttrSelectionRangeMax);
		return KeyboardViewBaseCreator::getAttributeNames (attributeNames);
	}
	AttrType getAttributeType (const std::string& attributeName) const override
	{
		if (attributeName == kAttrSelectionRangeMin)
			return kIntegerType;
		if (attributeName == kAttrSelectionRangeMax)
			return kIntegerType;
		return KeyboardViewBaseCreator::getAttributeType (attributeName);
	}
	bool getAttributeValue (CView* view, const std::string& attributeName, std::string& stringValue,
	                        const IUIDescription* desc) const override
	{
		auto kb = dynamic_cast<KeyboardViewRangeSelector*> (view);
		if (!kb)
			return false;
		if (attributeName == kAttrSelectionRangeMin)
		{
			stringValue = numberToString<int32_t> (kb->getSelectionMin ());
			return true;
		}
		if (attributeName == kAttrSelectionRangeMax)
		{
			stringValue = numberToString<int32_t> (kb->getSelectionMax ());
			return true;
		}
		return KeyboardViewBaseCreator::getAttributeValue (view, attributeName, stringValue, desc);
	}
	bool apply (CView* view, const UIAttributes& attributes,
	            const IUIDescription* desc) const override
	{
		auto kb = dynamic_cast<KeyboardViewRangeSelector*> (view);
		if (!kb)
			return false;
		auto selMin = static_cast<int32_t> (kb->getSelectionMin ());
		auto selMax = static_cast<int32_t> (kb->getSelectionMax ());
		attributes.getIntegerAttribute (kAttrSelectionRangeMin, selMin);
		attributes.getIntegerAttribute (kAttrSelectionRangeMax, selMax);
		kb->setSelectionMinMax (selMin, selMax);
		return KeyboardViewBaseCreator::apply (view, attributes, desc);
	}
};
KeyboardViewRangeSelectorCreator __gKeyboardViewRangeSelectorCreator;

//------------------------------------------------------------------------
} // VSTGUI
