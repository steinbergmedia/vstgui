/*
 *  uidescription test.cpp
 *  uidescription test
 *
 *  Created by Arne Scheffler on 11/26/09.
 *  Copyright 2009 Arne Scheffler. All rights reserved.
 *
 */

#include "uidescription test.h"
#include "pluginterfaces/base/ibstream.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include <cmath>

using namespace Steinberg;
using namespace Steinberg::Vst;

namespace VSTGUI {

//------------------------------------------------------------------------
FUID UIDescriptionTestProcessor::cid (0x49BAF003, 0xB44D455E, 0x9CBDE54F, 0x7FF2CBA1);
FUID UIDescriptionTestController::cid (0xF0FF3C24, 0x3F2F4C94, 0x84F6B6AE, 0xEF7BF28B);

//------------------------------------------------------------------------
enum {
	kPeakParam = 20,
};

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
class PeakParameter : public Parameter
{
public:
	PeakParameter (int32 flags, int32 id, const TChar* title);

	virtual void toString (ParamValue normValue, String128 string) const;
	virtual bool fromString (const TChar* string, ParamValue& normValue) const;
};

//------------------------------------------------------------------------
PeakParameter::PeakParameter (int32 flags, int32 id, const TChar* title)
{
	UString (info.title, USTRINGSIZE (info.title)).assign (title);
	
	info.flags = flags;
	info.id = id;
	info.stepCount = 0;
	info.defaultNormalizedValue = 0.5f;
	info.unitId = kRootUnitId;
	
	setNormalized (1.f);
}

//------------------------------------------------------------------------
void PeakParameter::toString (ParamValue normValue, String128 string) const
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
bool PeakParameter::fromString (const TChar* string, ParamValue& normValue) const
{
	return false;
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
tresult PLUGIN_API UIDescriptionBaseController::initialize (FUnknown* context)
{
	tresult res = EditController::initialize (context);
	if (res == kResultTrue)
	{
		// add processing parameters
		PeakParameter* peakParam = new PeakParameter (ParameterInfo::kIsReadOnly, kPeakParam, USTRING("Peak"));
		parameters.addParameter (peakParam);
	}
	return res;
}

//------------------------------------------------------------------------
Parameter* UIDescriptionBaseController::getParameterObject (ParamID tag)
{
	Parameter* param = EditController::getParameterObject (tag);
	if (param == 0)
	{
		param = uiParameters.getParameter (tag);
	}
	return param;
}

// make sure that our UI only parameters doesn't call the following three EditController methods: beginEdit, endEdit, performEdit
//------------------------------------------------------------------------
tresult UIDescriptionBaseController::beginEdit (ParamID tag)
{
	if (EditController::getParameterObject (tag))
		return EditController::beginEdit (tag);
	return kResultFalse;
}

//------------------------------------------------------------------------
tresult UIDescriptionBaseController::performEdit (ParamID tag, ParamValue valueNormalized)
{
	if (EditController::getParameterObject (tag))
		return EditController::performEdit (tag, valueNormalized);
	return kResultFalse;
}

//------------------------------------------------------------------------
tresult UIDescriptionBaseController::endEdit (ParamID tag)
{
	if (EditController::getParameterObject (tag))
		return EditController::endEdit (tag);
	return kResultFalse;
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
class ModalViewController : public DelegationController
{
public:
	ModalViewController (IController* controller, UIDescription* desc) : DelegationController (controller), desc (desc) {}

	CControlListener* getControlListener (const char* controlTagName) { return this; }
	void valueChanged (CControl* pControl)
	{
		if (pControl->getValue ())
		{
			switch (pControl->getTag ())
			{
				case 0:
				{
					CView* view = desc->createView ("ModalView", this);
					if (view)
					{
						CFrame* frame = pControl->getFrame ();
						CPoint center = frame->getViewSize ().getCenter ();
						CRect viewSize = view->getViewSize ();
						viewSize.offset (center.x - viewSize.getWidth () / 2, center.y - viewSize.getHeight () / 2);
						view->setViewSize (viewSize);
						view->setMouseableArea (viewSize);
						frame->setModalView (view);
						view->setAlphaValue (0.f);
						view->addAnimation ("AlphaFadeIn", new Animation::AlphaValueAnimation (1.f), new Animation::LinearTimingFunction (200));
						pControl->setValue (0);
						view->forget ();
					}
					break;
				}
				case 1:
				{
					pControl->getFrame ()->setModalView (0);
					break;
				}
			}
		}
	}

protected:
	UIDescription* desc;
};

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
UIDescriptionTestController::UIDescriptionTestController ()
{
}

//------------------------------------------------------------------------
tresult PLUGIN_API UIDescriptionTestController::initialize (FUnknown* context)
{
	tresult res = UIDescriptionBaseController::initialize (context);
	if (res == kResultTrue)
	{
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

//------------------------------------------------------------------------
IPlugView* PLUGIN_API UIDescriptionTestController::createView (FIDString name)
{
	if (strcmp (name, ViewType::kEditor) == 0)
	{
		return new VST3Editor (this, "view", "myEditor.uidesc");
	}
	return 0;
}

//------------------------------------------------------------------------
IController* UIDescriptionTestController::createSubController (const char* name, IUIDescription* description, VST3Editor* editor)
{
	if (strcmp (name, "ModalViewController") == 0)
	{
		return new ModalViewController (editor, dynamic_cast<UIDescription*> (description));
	}
	return 0;
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
UIDescriptionTestProcessor::UIDescriptionTestProcessor ()
{
	setControllerClass (UIDescriptionTestController::cid);
}

//------------------------------------------------------------------------
tresult PLUGIN_API UIDescriptionTestProcessor::initialize (FUnknown* context)
{
	tresult res = AudioEffect::initialize (context);
	if (res == kResultTrue)
	{
		addAudioInput (USTRING("Audio Input"), SpeakerArr::kStereo);
		addAudioOutput (USTRING("Audio Output"), SpeakerArr::kStereo);
	}
	return res;
}

//------------------------------------------------------------------------
tresult PLUGIN_API UIDescriptionTestProcessor::setBusArrangements (SpeakerArrangement* inputs, int32 numIns, SpeakerArrangement* outputs, int32 numOuts)
{
	if (numIns != 1 || numOuts != 1)
		return kResultFalse;
	if (inputs[0] != outputs[0])
		return kResultFalse;
	return AudioEffect::setBusArrangements (inputs, numIns, outputs, numOuts);
}

//------------------------------------------------------------------------
tresult PLUGIN_API UIDescriptionTestProcessor::process (ProcessData& data)
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

} // namespace VSTGUI

