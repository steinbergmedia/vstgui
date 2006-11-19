//-----------------------------------------------------------------------------
// VSTGUI: Graphical User Interface Framework for VST plugins : 
//
// Version 3.5       $Date: 2006-11-19 11:50:26 $
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// © 2004, Steinberg Media Technologies, All Rights Reserved
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
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A  PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#include "TutorialEditor.h"

//------------------------------------------------------------------------------------
class MyKnob : public CKnob
{
public:
	MyKnob (const CRect& size, CControlListener* listener, long tag, CBitmap* background, CBitmap* handle, CBitmap* highlightHandle);
	~MyKnob ();

	CMouseEventResult onMouseDown (CPoint &where, const long& buttons);
	CMouseEventResult onMouseUp (CPoint &where, const long& buttons);
	CMouseEventResult onMouseEntered (CPoint &where, const long& buttons);
	CMouseEventResult onMouseExited (CPoint &where, const long& buttons);
protected:
	CBitmap* handleBitmap;
	CBitmap* highlightHandleBitmap;
};

//------------------------------------------------------------------------------------
AEffGUIEditor* createEditor (AudioEffectX* effect)
{
	return new TutorialEditor (effect);
}

//------------------------------------------------------------------------------------
TutorialEditor::TutorialEditor (void* ptr)
: AEffGUIEditor (ptr)
{
}

//------------------------------------------------------------------------------------
bool TutorialEditor::open (void* ptr)
{
	//-- first we create the frame with a size of 300, 300 and set the background to white
	CRect frameSize (0, 0, 300, 300);
	CFrame* newFrame = new CFrame (frameSize, ptr, this);
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

//------------------------------------------------------------------------------------
void TutorialEditor::close ()
{
	//-- on close we need to delete the frame object
	CFrame* oldFrame = frame;
	frame = 0;
	delete oldFrame;
}

//------------------------------------------------------------------------------------
void TutorialEditor::valueChanged (CControl* pControl)
{
	//-- valueChanged is called whenever the user changes one of the controls in the User Interface (UI)
	effect->setParameterAutomated (pControl->getTag (), pControl->getValue ());
}

//------------------------------------------------------------------------------------
void TutorialEditor::setParameter (VstInt32 index, float value)
{
	//-- setParameter is called when the host automates one of the effects parameter.
	//-- The UI should reflect this state.
	if (frame && index < kNumParameters)
	{
		controls[index]->setValue (value);
	}
}

//------------------------------------------------------------------------------------
//-- MyKnob is just like CKnob except that it displays a different handle
//-- if the mouse is over it or if it is being tracked
//------------------------------------------------------------------------------------
MyKnob::MyKnob (const CRect& size, CControlListener* listener, long tag, CBitmap* background, CBitmap* handle, CBitmap* highlightHandle)
: CKnob (size, listener, tag, background, handle, CPoint (0, 0))
, handleBitmap (handle)
, highlightHandleBitmap (highlightHandle)
{
	handleBitmap->remember ();
	highlightHandleBitmap->remember ();
}

//------------------------------------------------------------------------------------
MyKnob::~MyKnob ()
{
	handleBitmap->forget ();
	highlightHandleBitmap->forget ();
}

//------------------------------------------------------------------------------------
CMouseEventResult MyKnob::onMouseDown (CPoint &where, const long& buttons)
{
	setHandleBitmap (highlightHandleBitmap);
	invalid ();
	return CKnob::onMouseDown (where, buttons);
}

//------------------------------------------------------------------------------------
CMouseEventResult MyKnob::onMouseUp (CPoint &where, const long& buttons)
{
	if (!where.isInside (size))
	{
		setHandleBitmap (handleBitmap);
		invalid ();
	}
	return CKnob::onMouseUp (where, buttons);
}

//------------------------------------------------------------------------------------
CMouseEventResult MyKnob::onMouseEntered (CPoint &where, const long& buttons)
{
	setHandleBitmap (highlightHandleBitmap);
	invalid ();
	return CKnob::onMouseEntered (where, buttons);
}

//------------------------------------------------------------------------------------
CMouseEventResult MyKnob::onMouseExited (CPoint &where, const long& buttons)
{
	setHandleBitmap (handleBitmap);
	invalid ();
	return CKnob::onMouseExited (where, buttons);
}

