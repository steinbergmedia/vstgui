// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __plugguieditor__
#define __plugguieditor__

#include "../vstgui.h"

//----------------------------------------------------------------------
struct ERect
{
	int16_t top;
	int16_t left;
	int16_t bottom;
	int16_t right;
};

//-----------------------------------------------------------------------------
// AEffGUIEditor Declaration
//-----------------------------------------------------------------------------
class PluginGUIEditor : public VSTGUIEditorInterface
{
public :

	PluginGUIEditor (void *pEffect);

	virtual ~PluginGUIEditor ();

	virtual void setParameter (int32_t index, float value) {} 
	virtual bool getRect (ERect **ppRect);
	virtual bool open (void *ptr);
	virtual void close () { systemWindow = 0; }
	virtual void idle ();
	virtual void draw (ERect *pRect);

	// wait (in ms)
	void wait (uint32_t ms);

	// get the current time (in ms)
	uint32_t getTicks ();

	// feedback to appli.
	virtual void doIdleStuff ();

	// get the effect attached to this editor
	void *getEffect () { return effect; }

	// get version of this VSTGUI
	int32_t getVstGuiVersion () { return (VSTGUI_VERSION_MAJOR << 16) + VSTGUI_VERSION_MINOR; }

	// set/get the knob mode
	virtual int32_t setKnobMode (int32_t val);
	virtual int32_t getKnobMode () const { return knobMode; }

	// get the CFrame object
	#if USE_NAMESPACE
	VSTGUI::CFrame *getFrame () { return frame; }
	#else
	CFrame *getFrame () { return frame; }
	#endif

	virtual void beginEdit (int32_t index) {}
	virtual void endEdit (int32_t index) {}

//---------------------------------------
protected:
	ERect   rect;

	void* effect;
	void* systemWindow;

private:
	uint32_t lLastTicks;
	bool inIdleStuff;

	static int32_t knobMode;
};

#endif
