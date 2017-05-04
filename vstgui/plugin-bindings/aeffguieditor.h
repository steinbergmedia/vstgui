// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

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
