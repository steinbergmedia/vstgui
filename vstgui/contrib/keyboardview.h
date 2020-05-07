//------------------------------------------------------------------------
// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE
// Flags : clang-format SMTGSequencer

#pragma once

#include "vstgui/lib/vstguifwd.h"
#include "vstgui/lib/ccolor.h"
#include "vstgui/lib/cview.h"
#include "vstgui/lib/dispatchlist.h"
#include "vstgui/lib/itouchevent.h"
#include <array>
#include <bitset>
#include <map>

namespace VSTGUI {

//------------------------------------------------------------------------
class KeyboardViewBase : public CView
{
public:
	using NoteIndex = int16_t;
	using NumNotes = uint8_t;

	static constexpr NumNotes MaxNotes = 128;

	enum class BitmapID
	{
		WhiteKeyPressed = 0,
		WhiteKeyUnpressed,
		BlackKeyPressed,
		BlackKeyUnpressed,
		WhiteKeyShadowLeft,
		WhiteKeyShadowRight,
		NumBitmaps
	};

	KeyboardViewBase ();

	void setKeyPressed (NoteIndex note, bool state);

	virtual void setKeyRange (NoteIndex startNote, NumNotes numKeys);
	NoteIndex getKeyRangeStart () const { return startNote; }
	NumNotes getNumKeys () const { return numKeys; }
	NumNotes getNumWhiteKeys () const;

	void setWhiteKeyWidth (CCoord width);
	void setBlackKeyWidth (CCoord width);
	void setBlackKeyHeight (CCoord height);
	void setLineWidth (CCoord width);
	CCoord getWhiteKeyWidth () const { return whiteKeyWidth; }
	CCoord getBlackKeyWidth () const { return blackKeyWidth; }
	CCoord getBlackKeyHeight () const { return blackKeyHeight; }
	CCoord getLineWidth () const { return lineWidth; }

	void setFrameColor (CColor color);
	void setFontColor (CColor color);
	void setWhiteKeyColor (CColor color);
	void setWhiteKeyPressedColor (CColor color);
	void setBlackKeyColor (CColor color);
	void setBlackKeyPressedColor (CColor color);
	CColor getFrameColor () const { return frameColor; }
	CColor getFontColor () const { return fontColor; }
	CColor getWhiteKeyColor () const { return whiteKeyColor; }
	CColor getWhiteKeyPressedColor () const { return whiteKeyPressedColor; }
	CColor getBlackKeyColor () const { return blackKeyColor; }
	CColor getBlackKeyPressedColor () const { return blackKeyPressedColor; }

	void setNoteNameFont (CFontDesc* font);
	CFontDesc* getNoteNameFont () const { return noteNameFont; }
	void setDrawNoteText (bool state);
	bool getDrawNoteText () const { return drawNoteText; }

	void setBitmap (BitmapID bID, CBitmap* bitmap);
	CBitmap* getBitmap (BitmapID bID) const;

	void setWhiteKeyBitmapInset (const CRect& inset); // TODO: uidesc
	void setBlackKeyBitmapInset (const CRect& inset); // TODO: uidesc

	const CRect& getNoteRect (NoteIndex note) const { return noteRectCache[note]; }
	bool isWhiteKey (NoteIndex note) const;

	void drawRect (CDrawContext* context, const CRect& dirtyRect) override;
	void setViewSize (const CRect& rect, bool invalid = true) override;
	bool sizeToFit () override;
//------------------------------------------------------------------------
protected:
	using NoteRectCache = std::array<CRect, MaxNotes>;

	void invalidNote (NoteIndex note);

	NoteIndex pointToNote (const CPoint& p, bool ignoreY) const;
	const NoteRectCache& getNoteRectCache () const { return noteRectCache; }

private:
	void drawNote (CDrawContext* context, CRect& rect, NoteIndex note, bool isWhite) const;
	CRect calcNoteRect (NoteIndex note) const;
	void updateNoteRectCache () const;
	void createBitmapCache ();

	using BitmapArray =
	    std::array<SharedPointer<CBitmap>, static_cast<size_t> (BitmapID::NumBitmaps)>;

	BitmapArray bitmaps;
	SharedPointer<CBitmap> whiteKeyBitmapCache;
	SharedPointer<CBitmap> blackKeyBitmapCache;
	SharedPointer<CFontDesc> noteNameFont;

	CRect whiteKeyBitmapInset;
	CRect blackKeyBitmapInset;

	CCoord whiteKeyWidth {30};
	CCoord blackKeyWidth {20};
	CCoord blackKeyHeight {20};
	CCoord lineWidth {1.};

	CColor frameColor {kBlackCColor};
	CColor fontColor {kBlackCColor};
	CColor whiteKeyColor {kWhiteCColor};
	CColor whiteKeyPressedColor {kGreyCColor};
	CColor blackKeyColor {kBlackCColor};
	CColor blackKeyPressedColor {kGreyCColor};

	NumNotes numKeys {88};
	NoteIndex startNote {21};
	bool drawNoteText {false};
	mutable bool noteRectCacheInvalid {true};
	mutable NoteRectCache noteRectCache;
	std::bitset<MaxNotes> keyPressed {};
};

class KeyboardViewRangeSelector;
//------------------------------------------------------------------------
struct IKeyboardViewKeyRangeChangedListener
{
	virtual void onKeyRangeChanged (KeyboardViewRangeSelector*) = 0;

	virtual ~IKeyboardViewKeyRangeChangedListener () noexcept = default;
};

//------------------------------------------------------------------------
class KeyboardViewRangeSelector : public KeyboardViewBase
{
public:
	struct Range
	{
		NoteIndex position;
		NumNotes length;
		Range (NoteIndex position = 0, NumNotes length = 0) : position (position), length (length)
		{
		}
		bool operator!= (const Range& r) const
		{
			return position != r.position || length != r.length;
		}
	};

	KeyboardViewRangeSelector () = default;

	void drawRect (CDrawContext* context, const CRect& dirtyRect) override;

	void setKeyRange (NoteIndex startNote, NumNotes numKeys) override;
	void setSelectionRange (const Range& range);
	void setSelectionMinMax (NumNotes minRange, NumNotes maxRange);
	const Range& getSelectionRange () const { return selectionRange; }
	NumNotes getSelectionMin () const { return rangeMin; }
	NumNotes getSelectionMax () const { return rangeMax; }
	NumNotes getNumWhiteKeysSelected () const;

	void registerKeyRangeChangedListener (IKeyboardViewKeyRangeChangedListener* listener);
	void unregisterKeyRangeChangedListener (IKeyboardViewKeyRangeChangedListener* listener);

//------------------------------------------------------------------------
private:
	DispatchList<IKeyboardViewKeyRangeChangedListener*> listeners;

	Range selectionRange {0, 12};
	NumNotes rangeMin {12};
	NumNotes rangeMax {24};

#if VSTGUI_TOUCH_EVENT_HANDLING
	void onTouchEvent (ITouchEvent& event) override;
	bool wantsMultiTouchEvents () const override;
	void onTouchBegin (const ITouchEvent::TouchPair& touch, ITouchEvent& event);
	void onTouchMove (const ITouchEvent::TouchPair& touch, ITouchEvent& event);

	enum TouchMode {kUnknown, kMoveRange, kChangeRangeFront, kChangeRangeBack};

	Range selectionRangeOnTouchStart;
	int32_t touchIds[2] {-1};
	TouchMode touchMode {kUnknown};
	NoteIndex touchStartNote[2];
#else
	CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseUp (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseCancel () override;

	Range moveStartRange;
	NoteIndex moveStartNote {-1};
#endif
};

//------------------------------------------------------------------------
struct IKeyboardViewPlayerDelegate
{
	using NoteIndex = KeyboardViewBase::NoteIndex;

	virtual int32_t onNoteOn (NoteIndex note, double xPos, double yPos) = 0;
	virtual void onNoteOff (NoteIndex note, int32_t noteID) = 0;

	virtual void onNoteModulation (int32_t noteID, double xPos, double yPos) = 0;

	virtual ~IKeyboardViewPlayerDelegate () noexcept = default;
};

//------------------------------------------------------------------------
struct KeyboardViewPlayerDelegate : public IKeyboardViewPlayerDelegate
{
	int32_t onNoteOn (NoteIndex note, double xPos, double yPos) override { return -1; }
	void onNoteOff (NoteIndex note, int32_t noteID) override {}
	void onNoteModulation (int32_t noteID, double xPos, double yPos) override {}
};

//------------------------------------------------------------------------
class KeyboardView : public KeyboardViewBase
{
public:
	KeyboardView ();
	void setDelegate (IKeyboardViewPlayerDelegate* inDelegate) { delegate = inDelegate; }

private:
	double calcYParameter (NoteIndex note, CCoord y) const;
	double calcXParameter (NoteIndex note, CCoord x) const;
#if VSTGUI_TOUCH_EVENT_HANDLING
	bool wantsMultiTouchEvents () const override { return true; }
	void onTouchEvent (ITouchEvent& event) override;
	void onTouchBegin (const ITouchEvent::TouchPair& touch, ITouchEvent& event);
	void onTouchMove (const ITouchEvent::TouchPair& touch, ITouchEvent& event);
	void onTouchEnd (const ITouchEvent::TouchPair& touch, ITouchEvent& event);

	struct NoteTouch
	{
		NoteIndex note;
		int32_t noteID;
		NoteTouch (NoteIndex note) : note (note), noteID (-1) {}
	};

	std::map<int32_t, NoteTouch> noteTouches;
#else
	void doNoteOff ();
	void doNoteOn (NoteIndex note, double yPos, double xPos);

	CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseUp (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseCancel () override;

	NoteIndex pressedNote {-1};
	int32_t noteID {-1};
#endif

	IKeyboardViewPlayerDelegate* delegate {nullptr};
};

//------------------------------------------------------------------------
} // VSTGUI
