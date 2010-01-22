/*
 *  uidescription test.cpp
 *  uidescription test
 *
 *  Created by Arne Scheffler on 11/26/09.
 *  Copyright 2009 Arne Scheffler. All rights reserved.
 *
 */

#include "public.sdk/source/vst/vst2wrapper/vst2wrapper.h"
#include "public.sdk/source/vst/vstaudioeffect.h"
#include "public.sdk/source/main/pluginfactoryvst3.h"
#include "pluginterfaces/base/ibstream.h"
#include "pluginterfaces/base/ustring.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "vstgui/uidescription/vst3editor.h"

using namespace Steinberg;

namespace VSTGUI {

enum {
	kPeakParam = 20,
};

//------------------------------------------------------------------------
class PeakParameter : public Steinberg::Vst::Parameter
{
public:
	PeakParameter (int32 flags, int32 id, const Steinberg::Vst::TChar* title);

	virtual void toString (Steinberg::Vst::ParamValue normValue, Steinberg::Vst::String128 string) const;
	virtual bool fromString (const Steinberg::Vst::TChar* string, Steinberg::Vst::ParamValue& normValue) const;
};

//------------------------------------------------------------------------
PeakParameter::PeakParameter (int32 flags, int32 id, const Steinberg::Vst::TChar* title)
{
	UString (info.title, USTRINGSIZE (info.title)).assign (title);
	
	info.flags = flags;
	info.id = id;
	info.stepCount = 0;
	info.defaultNormalizedValue = 0.5f;
	info.unitId = Steinberg::Vst::kRootUnitId;
	
	setNormalized (1.f);
}

//------------------------------------------------------------------------
void PeakParameter::toString (Steinberg::Vst::ParamValue normValue, Steinberg::Vst::String128 string) const
{
	String str;
	if (normValue > 0.0001)
	{
		str.printf ("%.3f", 20 * log10f ((float)normValue));
	}
	else
	{
		str.assign ("-");
		str.append (kInfiniteSymbol);
	}
	str.toWideString (kCP_Utf8);
	str.copyTo16 (string, 0, 128);
}

//------------------------------------------------------------------------
bool PeakParameter::fromString (const Steinberg::Vst::TChar* string, Steinberg::Vst::ParamValue& normValue) const
{
	return false;
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
class UIDescriptionTestController : public Steinberg::Vst::EditController
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
			PeakParameter* peakParam = new PeakParameter (Steinberg::Vst::ParameterInfo::kIsReadOnly, kPeakParam, USTRING("Peak"));
			parameters.addParameter (peakParam);
			
			// add ui parameters
			Steinberg::Vst::StringListParameter* slp = new Steinberg::Vst::StringListParameter (USTRING("TabController"), 20000);
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
		if (strcmp (name, Steinberg::Vst::ViewType::kEditor) == 0)
		{
			return new VST3Editor (this, "view", "myEditor.uidesc");
		}
		return 0;
	}

	Steinberg::Vst::Parameter* getParameterObject (Steinberg::Vst::ParamID tag)
	{
		Steinberg::Vst::Parameter* param = Steinberg::Vst::EditController::getParameterObject (tag);
		if (param == 0)
		{
			param = uiParameters.getParameter (tag);
		}
		return param;
	}

	// make sure that our UI only parameters doesn't call the following three EditController methods: beginEdit, endEdit, performEdit
	tresult beginEdit (Steinberg::Vst::ParamID tag)
	{
		if (Steinberg::Vst::EditController::getParameterObject (tag))
			return Steinberg::Vst::EditController::beginEdit (tag);
		return kResultFalse;
	}
	
	tresult performEdit (Steinberg::Vst::ParamID tag, Steinberg::Vst::ParamValue valueNormalized)
	{
		if (Steinberg::Vst::EditController::getParameterObject (tag))
			return EditController::performEdit (tag, valueNormalized);
		return kResultFalse;
	}
	
	tresult endEdit (Steinberg::Vst::ParamID tag)
	{
		if (Steinberg::Vst::EditController::getParameterObject (tag))
			return Steinberg::Vst::EditController::endEdit (tag);
		return kResultFalse;
	}

	static FUnknown* createInstance (void*) { return (IEditController*)new UIDescriptionTestController; }
	static FUID cid;
protected:
	Steinberg::Vst::ParameterContainer uiParameters;
};
FUID UIDescriptionTestController::cid (0xF0FF3C24, 0x3F2F4C94, 0x84F6B6AE, 0xEF7BF28B);

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
class UIDescriptionTestProcessor : public Steinberg::Vst::AudioEffect
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
			addAudioInput (USTRING("Audio Input"), Steinberg::Vst::SpeakerArr::kStereo);
			addAudioOutput (USTRING("Audio Output"), Steinberg::Vst::SpeakerArr::kStereo);
		}
		return res;
	}

	tresult PLUGIN_API setBusArrangements (Steinberg::Vst::SpeakerArrangement* inputs, int32 numIns, Steinberg::Vst::SpeakerArrangement* outputs, int32 numOuts)
	{
		if (numIns != 1 || numOuts != 1)
			return kResultFalse;
		if (inputs[0] != outputs[0])
			return kResultFalse;
		return Steinberg::Vst::AudioEffect::setBusArrangements (inputs, numIns, outputs, numOuts);
	}
	
	tresult PLUGIN_API process (Steinberg::Vst::ProcessData& data)
	{
		Steinberg::Vst::ParamValue peak = 0.;
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
			Steinberg::Vst::IParamValueQueue* queue = data.outputParameterChanges->addParameterData (kPeakParam, index);
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

//------------------------------------------------------------------------
AudioEffect* createEffectInstance (audioMasterCallback audioMaster)
{
	return Steinberg::Vst::Vst2Wrapper::create (GetPluginFactory (), UIDescriptionTestProcessor::cid, '????', audioMaster);
}
