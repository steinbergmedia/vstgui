// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "TutorialVST2Effect.h"
#include "../../plugin-bindings/aeffguieditor.h"

//------------------------------------------------------------------------------------
AudioEffect* createEffectInstance (audioMasterCallback audioMaster)
{
	return new TutorialVST2Effect (audioMaster);
}

//------------------------------------------------------------------------------------
TutorialVST2Effect::TutorialVST2Effect (audioMasterCallback audioMaster)
: AudioEffectX (audioMaster, 1, kNumParameters)
{
	setUniqueID (CCONST('G', 'U', 'I', '0'));
	setNumInputs (2);
	setNumOutputs (2);
	parameters[kLeftVolumeParameter] = 1.f;
	parameters[kRightVolumeParameter] = 1.f;

	extern AEffGUIEditor* createEditor (AudioEffectX*);
	setEditor (createEditor (this));
}

//------------------------------------------------------------------------------------
void TutorialVST2Effect::setParameter (VstInt32 index, float value)
{
	if (index < kNumParameters)
	{
		parameters[index] = value;
		if (editor)
			((AEffGUIEditor*)editor)->setParameter (index, value);
	}
}

//------------------------------------------------------------------------------------
float TutorialVST2Effect::getParameter (VstInt32 index)
{
	if (index < kNumParameters)
		return parameters[index];
	return 0.f;
}

//------------------------------------------------------------------------------------
void TutorialVST2Effect::processReplacing (float** inputs, float** outputs, VstInt32 sampleFrames)
{
	for (VstInt32 i = 0; i < sampleFrames; i++)
	{
		outputs[0][i] = inputs[0][i] * parameters[kLeftVolumeParameter];
		outputs[1][i] = inputs[1][i] * parameters[kRightVolumeParameter];
	}
}
