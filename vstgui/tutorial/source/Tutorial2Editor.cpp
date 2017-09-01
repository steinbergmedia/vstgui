// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

//-----------------------------------------------------------------------------------
//-- This is an advanced version of the first tutorial. It shows you how to inherit
//-- from CKnob and alter it's drawing by changing the handle bitmap if the mouse is
//-- over the control or if the knob is being tracked.
//-----------------------------------------------------------------------------------

#include "TutorialEditor.h"

//-----------------------------------------------------------------------------------
//-- This is the custom CKnob
//-- It takes another bitmap as parameter in its constructor which is the highlighted handle
//-----------------------------------------------------------------------------------
class MyKnob : public CKnob
{
public:
	MyKnob (const CRect& size, CControlListener* listener, int32_t tag, CBitmap* background, CBitmap* handle, CBitmap* highlightHandle);
	~MyKnob ();

	CMouseEventResult onMouseDown (CPoint &where, const CButtonState& buttons);
	CMouseEventResult onMouseUp (CPoint &where, const CButtonState& buttons);
	CMouseEventResult onMouseEntered (CPoint &where, const CButtonState& buttons);
	CMouseEventResult onMouseExited (CPoint &where, const CButtonState& buttons);
protected:
	CBitmap* handleBitmap;
	CBitmap* highlightHandleBitmap;
};

//-----------------------------------------------------------------------------------
AEffGUIEditor* createEditor (AudioEffectX* effect)
{
	return new TutorialEditor (effect);
}

//-----------------------------------------------------------------------------------
TutorialEditor::TutorialEditor (void* ptr)
: AEffGUIEditor (ptr)
{
	rect.left = 0;
	rect.top = 0;
	rect.right = 300;
	rect.bottom = 300;
}

//-----------------------------------------------------------------------------------
bool TutorialEditor::open (void* ptr)
{
	//-- first we create the frame with a size of 300, 300 and set the background to white
	CRect frameSize (rect.left, rect.top, rect.right, rect.bottom);
	CFrame* newFrame = new CFrame (frameSize, this);
	newFrame->open (ptr);
	newFrame->setBackgroundColor (kWhiteCColor);

	//-- load some bitmaps we need
	CBitmap* background = new CBitmap ("KnobBackground.png");
	CBitmap* handle = new CBitmap ("KnobHandle.png");
	CBitmap* handleHighlight = new CBitmap ("KnobHandleHighlight.png");

	//-- creating a knob and adding it to the frame
	CRect r (0, 0, background->getWidth (), background->getHeight ());
	CKnob* knob1 = new MyKnob (r, this, kLeftVolumeParameter, background, handle, handleHighlight);
	newFrame->addView (knob1);

	//-- creating another knob, we are offsetting the rect, so that the knob is next to the previous knob
	r.offset (background->getWidth () + 5, 0);
	CKnob* knob2 = new MyKnob (r, this, kRightVolumeParameter, background, handle, handleHighlight);
	newFrame->addView (knob2);

	//-- forget the bitmaps
	background->forget ();
	handle->forget ();

	//-- remember our controls so that we can sync them with the state of the effect
	controls[kLeftVolumeParameter] = knob1;
	controls[kRightVolumeParameter] = knob2;

	//-- set the member frame to our frame
	frame = newFrame;

	//-- sync parameters
	for (int i = 0; i < kNumParameters; i++)
		setParameter (i, effect->getParameter (i));
	return true;
}

//-----------------------------------------------------------------------------------
void TutorialEditor::close ()
{
	//-- on close we need to delete the frame object.
	//-- once again we make sure that the member frame variable is set to zero before we delete it
	//-- so that calls to setParameter won't crash.
	CFrame* oldFrame = frame;
	frame = 0;
	oldFrame->forget ();
}

//-----------------------------------------------------------------------------------
void TutorialEditor::valueChanged (CControl* pControl)
{
	//-- valueChanged is called whenever the user changes one of the controls in the User Interface (UI)
	effect->setParameterAutomated (pControl->getTag (), pControl->getValue ());
}

//-----------------------------------------------------------------------------------
void TutorialEditor::setParameter (VstInt32 index, float value)
{
	//-- setParameter is called when the host automates one of the effects parameter.
	//-- The UI should reflect this state.
	if (frame && index < kNumParameters)
	{
		controls[index]->setValue (value);
	}
}

//-----------------------------------------------------------------------------------
//-- MyKnob is just like CKnob except that it displays a different handle
//-- if the mouse is over it or if it is being tracked
//-----------------------------------------------------------------------------------
MyKnob::MyKnob (const CRect& size, CControlListener* listener, int32_t tag, CBitmap* background, CBitmap* handle, CBitmap* highlightHandle)
: CKnob (size, listener, tag, background, handle, CPoint (0, 0))
, handleBitmap (handle)
, highlightHandleBitmap (highlightHandle)
{
	//-- we must remember both handle bitmaps, otherwise they might get destroyed in CKnob::setHandleBitmap
	handleBitmap->remember ();
	highlightHandleBitmap->remember ();
}

//-----------------------------------------------------------------------------------
MyKnob::~MyKnob ()
{
	//-- as we remembered the bitmaps, we also must forget them if we don't need them anymore
	handleBitmap->forget ();
	highlightHandleBitmap->forget ();
}

//-----------------------------------------------------------------------------------
CMouseEventResult MyKnob::onMouseDown (CPoint &where, const CButtonState& buttons)
{
	//-- we are being tracked so change the handle bitmap to the highlighted one
	setHandleBitmap (highlightHandleBitmap);

	//-- make sure we will get redrawn
	invalid ();

	//-- all other mouse handling stuff is done in the parent class
	return CKnob::onMouseDown (where, buttons);
}

//-----------------------------------------------------------------------------------
CMouseEventResult MyKnob::onMouseUp (CPoint &where, const CButtonState& buttons)
{
	//-- if the mouse is not over the knob
	if (!where.isInside (size))
	{
		//-- we change the handle to the default one
		setHandleBitmap (handleBitmap);

		//-- and again make sure we will get redrawn
		invalid ();
	}
	return CKnob::onMouseUp (where, buttons);
}

//-----------------------------------------------------------------------------------
CMouseEventResult MyKnob::onMouseEntered (CPoint &where, const CButtonState& buttons)
{
	//-- the mouse entered our knob, so we change the handle to the highlighted one
	setHandleBitmap (highlightHandleBitmap);

	//-- we must get redrawn
	invalid ();

	return CKnob::onMouseEntered (where, buttons);
}

//-----------------------------------------------------------------------------------
CMouseEventResult MyKnob::onMouseExited (CPoint &where, const CButtonState& buttons)
{
	//-- the mouse exited our knob, so change the handle to the default one
	setHandleBitmap (handleBitmap);

	//-- we must get redrawn
	invalid ();

	return CKnob::onMouseExited (where, buttons);
}

