//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 2.2         Date : 20/11/01
//
// © 2003, Steinberg Media Technologies, All Rights Reserved
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// © 2003, Steinberg Media Technologies, All Rights Reserved
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

#ifndef __aeffguieditor__
#define __aeffguieditor__

#ifndef __AEffEditor__
#include "AEffEditor.hpp"
#endif

#ifndef __vstgui__
#include "vstgui.h"
#endif

//-----------------------------------------------------------------------------
// AEffGUIEditor Declaration
//-----------------------------------------------------------------------------
class AEffGUIEditor : public AEffEditor
{
public :

	AEffGUIEditor (AudioEffect *pEffect);

	virtual ~AEffGUIEditor ();

	virtual void setParameter (long index, float value) { postUpdate (); } 
	virtual long getRect (ERect **ppRect);
	virtual long open (void *ptr);
	virtual void idle ();
	virtual void draw (ERect *pRect);

	#if VST_2_1_EXTENSIONS
	virtual long onKeyDown (VstKeyCode &keyCode);
	virtual long onKeyUp (VstKeyCode &keyCode);
	#endif

	#if MAC
	virtual long mouse (long x, long y);
	#endif

	// wait (in ms)
	void wait (unsigned long ms);

	// get the current time (in ms)
	unsigned long getTicks ();

	// feedback to appli.
	virtual void doIdleStuff ();

	// get the effect attached to this editor
	AudioEffect *getEffect () { return effect; }

	// get version of this VSTGUI
	long getVstGuiVersion () { return (VSTGUI_VERSION_MAJOR << 16) + VSTGUI_VERSION_MINOR; }

	// set/get the knob mode
	virtual long setKnobMode (int val);
	static  long getKnobMode () { return knobMode; }

	virtual bool onWheel (float distance);

	// get the CFrame object
	#if USE_NAMESPACE
	VSTGUI::CFrame *getFrame () { return frame; }
	#else
	CFrame *getFrame () { return frame; }
	#endif

//---------------------------------------
protected:
	ERect   rect;

	#if USE_NAMESPACE
	VSTGUI::CFrame *frame;
	#else
	CFrame *frame;
	#endif

private:
	unsigned long lLastTicks;
	bool inIdleStuff;

	static long knobMode;
};

#endif
