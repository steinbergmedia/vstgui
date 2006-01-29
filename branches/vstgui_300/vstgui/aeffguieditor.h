//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 3.0       Date : 30/06/04
//
//-----------------------------------------------------------------------------
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

#ifndef __aeffguieditor__
#define __aeffguieditor__

#ifndef __aeffeditor__
#include "aeffeditor.h"
#endif

#ifndef __vstgui__
#include "vstgui.h"
#endif

//-----------------------------------------------------------------------------
// AEffGUIEditor Declaration
//-----------------------------------------------------------------------------
class AEffGUIEditor : public AEffEditor
{
public:
//-----------------------------------------------------------------------------
	AEffGUIEditor (AudioEffect* effect);
	~AEffGUIEditor ();

	// get the CFrame object
	#if USE_NAMESPACE
	VSTGUI::CFrame* getFrame () { return frame; }
	#else
	CFrame* getFrame () { return frame; }
	#endif

	virtual void setParameter (VstInt32 index, float value);
	virtual void beginEdit (VstInt32 index);
	virtual void endEdit (VstInt32 index);

	// feedback to application
	virtual void doIdleStuff ();

	// wait (in ms)
	void wait (unsigned int ms);

	// get the current time (in ms)
	unsigned int getTicks ();

	// get version of this VSTGUI
	static int getVstGuiVersion () { return (VSTGUI_VERSION_MAJOR << 16) + VSTGUI_VERSION_MINOR; }

	// get the knob mode
	static VstInt32 getKnobMode () { return knobMode; }
//-----------------------------------------------------------------------------
// AEffEditor overrides:
//-----------------------------------------------------------------------------
	bool getRect (ERect** rect);
	void idle ();
	
	#if MAC
	void DECLARE_VST_DEPRECATED (draw) (ERect* rect);
	VstInt32 DECLARE_VST_DEPRECATED (mouse) (VstInt32 x, VstInt32 y);
	#endif

	#if VST_2_1_EXTENSIONS
	bool onKeyDown (VstKeyCode& keyCode);
	bool onKeyUp (VstKeyCode& keyCode);
	bool onWheel (float distance);
	bool setKnobMode (VstInt32 val);
	#endif
//-----------------------------------------------------------------------------
protected:
	ERect rect;
	unsigned int lLastTicks;
	bool inIdleStuff;
	static VstInt32 knobMode;
	#if USE_NAMESPACE
	VSTGUI::CFrame* frame;
	#else
	CFrame* frame;
	#endif
};

#endif
