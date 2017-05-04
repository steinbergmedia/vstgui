// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "subcontrollertest.h"
#include "base/source/fobject.h"
#include "vstgui/uidescription/delegationcontroller.h"

using namespace Steinberg;
using namespace Steinberg::Vst;

namespace VSTGUI {

//------------------------------------------------------------------------
FUID SubControllerTestProcessor::cid (0x11E3B945, 0x7F274CC3, 0x9BC49338, 0xE9A61611);
FUID SubControllerTestController::cid (0x19960BD6, 0x4442464C, 0x821E17E2, 0xCB2903DD);

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
SubControllerTestProcessor::SubControllerTestProcessor ()
{
	setControllerClass (SubControllerTestController::cid);
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
enum {
	kFrequencyTag = 100,
	kResonanceTag
};

//------------------------------------------------------------------------
class TestSubController : public DelegationController, public FObject
{
public:
	TestSubController (IController* controller, Parameter* switchParameter);
	~TestSubController ();

	CView* verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description) override;
	
	void PLUGIN_API update (FUnknown* changedUnknown, int32 message) override;
	OBJ_METHODS(TestSubController, FObject)
protected:
	Parameter* switchParameter;
	std::list<CControl*> freqControls;
	std::list<CControl*> resoControls;
};

//------------------------------------------------------------------------
tresult PLUGIN_API SubControllerTestController::initialize (FUnknown* context)
{
	tresult res = UIDescriptionBaseController::initialize (context);
	if (res == kResultTrue)
	{
		parameters.addParameter (new RangeParameter (USTRING("Frequency 1"), kFrequencyTag, 0, 50, 22000, 1000));
		parameters.addParameter (new RangeParameter (USTRING("Resonance 1"), kResonanceTag, 0, 0, 1, 0));
		parameters.addParameter (new RangeParameter (USTRING("Frequency 2"), kFrequencyTag+2, 0, 50, 22000, 1000));
		parameters.addParameter (new RangeParameter (USTRING("Resonance 2"), kResonanceTag+2, 0, 0, 1, 0));

		getParameterObject (kFrequencyTag)->setPrecision (0);
		getParameterObject (kFrequencyTag+2)->setPrecision (0);

		// add ui parameters
		StringListParameter* slp = new StringListParameter (USTRING("Filter Switch"), 20000);
		slp->appendString (USTRING("Filter 1"));
		slp->appendString (USTRING("Filter 2"));
		uiParameters.addParameter (slp);
	}
	return res;
}

//------------------------------------------------------------------------
IPlugView* PLUGIN_API SubControllerTestController::createView (FIDString name)
{
	if (strcmp (name, ViewType::kEditor) == 0)
	{
		return new VST3Editor (this, "view", "SubControllerTest.uidesc");
	}
	return 0;
}

//------------------------------------------------------------------------
IController* SubControllerTestController::createSubController (const char* name, const IUIDescription* description, VST3Editor* editor)
{
	if (ConstString (name) == "FilterController")
	{
		return new TestSubController (editor, uiParameters.getParameter (20000));
	}
	return 0;
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
TestSubController::TestSubController (IController* controller, Parameter* switchParameter)
: DelegationController (controller)
, switchParameter (switchParameter)
{
	switchParameter->addDependent (this);
}

//------------------------------------------------------------------------
TestSubController::~TestSubController ()
{
	switchParameter->removeDependent (this);
}

//------------------------------------------------------------------------
CView* TestSubController::verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description)
{
	view = DelegationController::verifyView (view, attributes, description);
	if (view)
	{
		CControl* control = dynamic_cast<CControl*> (view);
		if (control)
		{
			switch (control->getTag ())
			{
				case kFrequencyTag:
				{
					freqControls.push_back (control);
					update (switchParameter, kChanged);
					break;
				}
				case kResonanceTag:
				{
					resoControls.push_back (control);
					update (switchParameter, kChanged);
					break;
				}
			}
		}
	}
	return view;
}

//------------------------------------------------------------------------
void PLUGIN_API TestSubController::update (FUnknown* changedUnknown, int32 message)
{
	if (message == kChanged)
	{
		FUnknownPtr<Parameter> parameter (changedUnknown);
		if (parameter)
		{
			int32 value = static_cast<int32> (parameter->toPlain (parameter->getNormalized ()));
			if (value == 0)
			{
				std::list<CControl*>::iterator it = freqControls.begin ();
				while (it != freqControls.end ())
				{
					if ((*it)->getTag () != kFrequencyTag)
						(*it)->setTag (kFrequencyTag);
					it++;
				}
				std::list<CControl*>::iterator it2 = resoControls.begin ();
				while (it2 != resoControls.end ())
				{
					if ((*it2)->getTag () != kResonanceTag)
						(*it2)->setTag (kResonanceTag);
					it2++;
				}
			}
			else if (value == 1)
			{
				std::list<CControl*>::iterator it = freqControls.begin ();
				while (it != freqControls.end ())
				{
					if ((*it)->getTag () != kFrequencyTag+2)
						(*it)->setTag (kFrequencyTag+2);
					it++;
				}
				std::list<CControl*>::iterator it2 = resoControls.begin ();
				while (it2 != resoControls.end ())
				{
					if ((*it2)->getTag () != kResonanceTag+2)
						(*it2)->setTag (kResonanceTag+2);
					it2++;
				}
			}
		}
	}
}

} // namespace
