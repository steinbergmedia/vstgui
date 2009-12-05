/*
 *  uidescription test.cpp
 *  uidescription test
 *
 *  Created by Arne Scheffler on 11/26/09.
 *  Copyright 2009 Arne Scheffler. All rights reserved.
 *
 */

//#define INIT_CLASS_IID

#include "public.sdk/source/vst/vstaudioeffect.h"
#include "public.sdk/source/main/pluginfactoryvst3.h"
#include "pluginterfaces/base/ibstream.h"
#include "pluginterfaces/base/ustring.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "vstgui/uidescription/vst3editor.h"

using namespace Steinberg;
using namespace Steinberg::Vst;

namespace VSTGUI {

enum {
	kPeakParam = 20,
};

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
class UIDescriptionTestController : public EditController
{
public:
	UIDescriptionTestController ()
	{
	}
	
	tresult PLUGIN_API initialize (FUnknown* context)
	{
		tresult res = EditController::initialize (context);
		if (res == kResultTrue)
		{
			// add processing parameters
			parameters.addParameter (USTRING("Peak"), 0, 0, 0, ParameterInfo::kIsReadOnly, kPeakParam);
			
			// add ui parameters
			StringListParameter* slp = new StringListParameter (USTRING("TabController"), 20000);
			slp->appendString (USTRING("Tab1"));
			slp->appendString (USTRING("Tab2"));
			slp->appendString (USTRING("Tab3"));
			slp->appendString (USTRING("Tab4"));
			slp->appendString (USTRING("Tab5"));
			uiParameters.addParameter (slp);
		}
		return res;
	}

	IPlugView* PLUGIN_API createView (FIDString name)
	{
		if (strcmp (name, ViewType::kEditor) == 0)
		{
			#if DEBUG
			return new VST3Editor (this, "view", "myEditor.uidesc", true);
			#else
			return new VST3Editor (this, "view", "myEditor.uidesc", false);
			#endif
		}
		return 0;
	}

	Parameter* getParameterObject (ParamID tag)
	{
		Parameter* param = EditController::getParameterObject (tag);
		if (param == 0)
		{
			param = uiParameters.getParameter (tag);
		}
		return param;
	}

	// make sure that our UI only parameters doesn't call the following three EditController methods: beginEdit, endEdit, performEdit
	tresult beginEdit (ParamID tag)
	{
		if (EditController::getParameterObject (tag))
			return EditController::beginEdit (tag);
		return kResultFalse;
	}
	
	tresult performEdit (ParamID tag, ParamValue valueNormalized)
	{
		if (EditController::getParameterObject (tag))
			return EditController::performEdit (tag, valueNormalized);
		return kResultFalse;
	}
	
	tresult endEdit (ParamID tag)
	{
		if (EditController::getParameterObject (tag))
			return EditController::endEdit (tag);
		return kResultFalse;
	}

	static FUnknown* createInstance (void*) { return (IEditController*)new UIDescriptionTestController; }
	static FUID cid;
protected:
	ParameterContainer uiParameters;
};
FUID UIDescriptionTestController::cid (0xF0FF3C24, 0x3F2F4C94, 0x84F6B6AE, 0xEF7BF28B);

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
class UIDescriptionTestProcessor : public AudioEffect
{
public:
	UIDescriptionTestProcessor ()
	{
		setControllerClass (UIDescriptionTestController::cid);
	}
	
	tresult PLUGIN_API initialize (FUnknown* context)
	{
		tresult res = AudioEffect::initialize (context);
		if (res == kResultTrue)
		{
			addAudioInput (USTRING("Audio Input"), SpeakerArr::kStereo);
			addAudioOutput (USTRING("Audio Output"), SpeakerArr::kStereo);
		}
		return res;
	}

	tresult PLUGIN_API setBusArrangements (SpeakerArrangement* inputs, int32 numIns, SpeakerArrangement* outputs, int32 numOuts)
	{
		if (numIns != 1 || numOuts != 1)
			return kResultFalse;
		if (inputs[0] != outputs[0])
			return kResultFalse;
		return AudioEffect::setBusArrangements (inputs, numIns, outputs, numOuts);
	}
	
	tresult PLUGIN_API process (ProcessData& data)
	{
		ParamValue peak = 0.;
		for (int32 sample = 0; sample < data.numSamples; sample++)
		{
			for (int32 channel = 0; channel < data.inputs[0].numChannels; channel++)
			{
				float value = data.inputs[0].channelBuffers32[channel][sample];
				data.outputs[0].channelBuffers32[channel][sample] = value;
				value = fabs (value);
				if (value > peak)
					peak = value;
			}
		}
		if (data.outputParameterChanges)
		{
			int32 index;
			IParamValueQueue* queue = data.outputParameterChanges->addParameterData (kPeakParam, index);
			if (queue)
				queue->addPoint (0, peak, index);
		}
		return kResultTrue;
	}

	static FUnknown* createInstance (void*) { return (IAudioProcessor*)new UIDescriptionTestProcessor; }
	static FUID cid;
};
FUID UIDescriptionTestProcessor::cid (0x49BAF003, 0xB44D455E, 0x9CBDE54F, 0x7FF2CBA1);


} // namespace VSTGUI

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
BEGIN_FACTORY("VSTGUI", "", "", PFactoryInfo::kUnicode)

	DEF_CLASS2 (INLINE_UID_FROM_FUID(UIDescriptionTestProcessor::cid),
				PClassInfo::kManyInstances,
				kVstAudioEffectClass,
				"VSTGUI UIDescription Test",
				Vst::kDistributable,
				"Fx",
				"1.0.0",
				kVstVersionString,
				UIDescriptionTestProcessor::createInstance)
				
	DEF_CLASS2 (INLINE_UID_FROM_FUID(UIDescriptionTestController::cid),
				PClassInfo::kManyInstances,
				kVstComponentControllerClass,
				"VSTGUI UIDescription Test",
				Vst::kDistributable,
				"Fx",
				"1.0.0",
				kVstVersionString,
				UIDescriptionTestController::createInstance)
				
END_FACTORY

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
void* hInstance = 0;

//------------------------------------------------------------------------
bool InitModule ()   
{
	extern void* moduleHandle;
	hInstance = moduleHandle;

	return true; 
}

//------------------------------------------------------------------------
bool DeinitModule ()
{
	return true; 
}
