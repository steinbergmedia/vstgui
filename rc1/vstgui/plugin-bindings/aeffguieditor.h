//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 4.3
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2015, Steinberg Media Technologies, All Rights Reserved
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
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
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
#include "public.sdk/source/vst2.x/aeffeditor.h"
#endif

#ifndef __audioeffectx__
#include "public.sdk/source/vst2.x/audioeffectx.h"
#endif

#include "../vstgui.h"

//-----------------------------------------------------------------------------
// AEffGUIEditor Declaration
//-----------------------------------------------------------------------------
class AEffGUIEditor : public AEffEditor, public VSTGUIEditorInterface
{
public :

	AEffGUIEditor (void* pEffect);

	virtual ~AEffGUIEditor ();

	virtual void setParameter (VstInt32 index, float value) {} 
	virtual bool getRect (ERect** ppRect);
	virtual bool open (void* ptr);
	virtual void idle ();
	virtual void draw (ERect* pRect);

	#if VST_2_1_EXTENSIONS
	virtual bool onKeyDown (VstKeyCode& keyCode);
	virtual bool onKeyUp (VstKeyCode& keyCode);
	#endif

	// wait (in ms)
	void wait (uint32_t ms);

	// get the current time (in ms)
	uint32_t getTicks ();

	// feedback to appli.
	virtual void doIdleStuff ();

	// get the effect attached to this editor
	AudioEffect* getEffect () { return effect; }

	// get version of this VSTGUI
	int32_t getVstGuiVersion () { return (VSTGUI_VERSION_MAJOR << 16) + VSTGUI_VERSION_MINOR; }

	// set/get the knob mode
	virtual bool setKnobMode (int32_t val);
	virtual int32_t getKnobMode () const { return knobMode; }

	virtual bool beforeSizeChange (const CRect& newSize, const CRect& oldSize);

	virtual bool onWheel (float distance);

#if VST_2_1_EXTENSIONS
	virtual void beginEdit (int32_t index) { ((AudioEffectX*)effect)->beginEdit (index); }
	virtual void endEdit (int32_t index)   { ((AudioEffectX*)effect)->endEdit (index); }
#endif

//---------------------------------------
protected:
	ERect   rect;

private:
	uint32_t lLastTicks;
	bool inIdleStuff;

	static int32_t knobMode;
};

#endif
