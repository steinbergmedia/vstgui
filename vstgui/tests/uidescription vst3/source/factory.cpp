/*
 *  factory.cpp
 *  uidescription test
 *
 *  Created by Arne Scheffler on 2/12/10.
 *  Copyright 2010 Arne Scheffler. All rights reserved.
 *
 */

#include "public.sdk/source/vst/vst2wrapper/vst2wrapper.h"
#include "uidescription test.h"
#include "subcontrollertest.h"
#include "animationtest.h"
#include "graphicstest.h"
#include "openglviewtest.h"
#include "public.sdk/source/main/pluginfactoryvst3.h"

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
				
	DEF_CLASS2 (INLINE_UID_FROM_FUID(SubControllerTestProcessor::cid),
				PClassInfo::kManyInstances,
				kVstAudioEffectClass,
				"VSTGUI UIDescription SubController Test",
				Vst::kDistributable,
				"Fx",
				"1.0.0",
				kVstVersionString,
				SubControllerTestProcessor::createInstance)
				
	DEF_CLASS2 (INLINE_UID_FROM_FUID(SubControllerTestController::cid),
				PClassInfo::kManyInstances,
				kVstComponentControllerClass,
				"VSTGUI UIDescription SubController Test",
				Vst::kDistributable,
				"Fx",
				"1.0.0",
				kVstVersionString,
				SubControllerTestController::createInstance)
				
	DEF_CLASS2 (INLINE_UID_FROM_FUID(AnimationTestProcessor::cid),
				PClassInfo::kManyInstances,
				kVstAudioEffectClass,
				"VSTGUI UIDescription Animation Test",
				Vst::kDistributable,
				"Fx",
				"1.0.0",
				kVstVersionString,
				AnimationTestProcessor::createInstance)
				
	DEF_CLASS2 (INLINE_UID_FROM_FUID(AnimationTestController::cid),
				PClassInfo::kManyInstances,
				kVstComponentControllerClass,
				"VSTGUI UIDescription Animation Test",
				Vst::kDistributable,
				"Fx",
				"1.0.0",
				kVstVersionString,
				AnimationTestController::createInstance)
				
	DEF_CLASS2 (INLINE_UID_FROM_FUID(GraphicsTestProcessor::cid),
				PClassInfo::kManyInstances,
				kVstAudioEffectClass,
				"VSTGUI UIDescription Graphics Test",
				Vst::kDistributable,
				"Fx",
				"1.0.0",
				kVstVersionString,
				GraphicsTestProcessor::createInstance)
				
	DEF_CLASS2 (INLINE_UID_FROM_FUID(GraphicsTestController::cid),
				PClassInfo::kManyInstances,
				kVstComponentControllerClass,
				"VSTGUI UIDescription Graphics Test",
				Vst::kDistributable,
				"Fx",
				"1.0.0",
				kVstVersionString,
				GraphicsTestController::createInstance)
				
	DEF_CLASS2 (INLINE_UID_FROM_FUID(OpenGLViewTestProcessor::cid),
				PClassInfo::kManyInstances,
				kVstAudioEffectClass,
				"VSTGUI UIDescription OpenGLView Test",
				Vst::kDistributable,
				"Fx",
				"1.0.0",
				kVstVersionString,
				OpenGLViewTestProcessor::createInstance)
				
	DEF_CLASS2 (INLINE_UID_FROM_FUID(OpenGLViewTestController::cid),
				PClassInfo::kManyInstances,
				kVstComponentControllerClass,
				"VSTGUI UIDescription OpenGLView Test",
				Vst::kDistributable,
				"Fx",
				"1.0.0",
				kVstVersionString,
				OpenGLViewTestController::createInstance)
				
END_FACTORY

//------------------------------------------------------------------------
bool InitModule ()   
{
	return true; 
}

//------------------------------------------------------------------------
bool DeinitModule ()
{
	return true; 
}

//------------------------------------------------------------------------
::AudioEffect* createEffectInstance (::audioMasterCallback audioMaster)
{
	return Steinberg::Vst::Vst2Wrapper::create (GetPluginFactory (), UIDescriptionTestProcessor::cid, 0, audioMaster);
}
