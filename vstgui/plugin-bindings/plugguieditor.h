// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __plugguieditor__
#define __plugguieditor__

#include "../vstgui.h"

namespace VSTGUI {

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
	~PluginGUIEditor () override;

	virtual void setParameter (int32_t index, float value) {} 
	virtual bool getRect (ERect **ppRect);
	virtual bool open (void *ptr);
	virtual void close () { systemWindow = nullptr; }
	virtual void idle ();
	virtual void draw (ERect *pRect);

	// wait (in ms)
	void wait (uint32_t ms);

	// get the current time (in ms)
	uint32_t getTicks ();

	// feedback to appli.
	void doIdleStuff () override;

	// get the effect attached to this editor
	void *getEffect () { return effect; }

	// get version of this VSTGUI
	int32_t getVstGuiVersion () { return (VSTGUI_VERSION_MAJOR << 16) + VSTGUI_VERSION_MINOR; }

	// set/get the knob mode
	virtual int32_t setKnobMode (int32_t val);
	int32_t getKnobMode () const override { return knobMode; }

	void beginEdit (int32_t index) override {}
	void endEdit (int32_t index) override {}

//---------------------------------------
protected:
	ERect   rect;

	void* effect;
	void* systemWindow;

private:
	uint32_t lLastTicks;

	static int32_t knobMode;
};

} // VSTGUI

#endif
