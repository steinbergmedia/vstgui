//
//  ZoomTest.cpp
//  uidescription test
//
//  Created by Arne Scheffler on 26/06/14.
//
//

#include "zoomtest.h"
#include "vstgui/plugin-bindings/vst3groupcontroller.h"
#include <vector>

using namespace Steinberg;
using namespace Steinberg::Vst;

namespace VSTGUI {

//------------------------------------------------------------------------
FUID ZoomTestProcessor::cid (0x2DEB807D, 0xB2C542D7, 0xA7D8421A, 0x835D5860);
FUID ZoomTestController::cid (0xB3E89625, 0x24B142F6, 0x8EA9F6CB, 0xFC4196F0);

struct ZoomFactor {
	const tchar* title;
	double factor;
	
	ZoomFactor (const tchar* title, double factor) : title (title), factor (factor) {}
};

typedef std::vector<ZoomFactor> ZoomFactorVector;
static ZoomFactorVector zoomFactors;

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
ZoomTestProcessor::ZoomTestProcessor ()
{
	setControllerClass (ZoomTestController::cid);
	if (zoomFactors.empty ())
	{
		zoomFactors.push_back (ZoomFactor (STR("50%"), 0.5));
		zoomFactors.push_back (ZoomFactor (STR("75%"), 0.75));
		zoomFactors.push_back (ZoomFactor (STR("100%"), 1.));
		zoomFactors.push_back (ZoomFactor (STR("125%"), 1.25));
		zoomFactors.push_back (ZoomFactor (STR("150%"), 1.5));
		zoomFactors.push_back (ZoomFactor (STR("175%"), 1.75));
		zoomFactors.push_back (ZoomFactor (STR("200%"), 2.));
	}
}

//------------------------------------------------------------------------
tresult PLUGIN_API ZoomTestController::initialize (FUnknown* context)
{
	tresult res = UIDescriptionBaseController::initialize (context);
	if (res == kResultTrue)
	{
		StringListParameter* zoomParameter = new StringListParameter (STR ("Zoom"), 1000);
		for (ZoomFactorVector::const_iterator it = zoomFactors.begin(), end = zoomFactors.end(); it != end; ++it)
		{
			zoomParameter->appendString (it->title);
		}
		zoomParameter->setNormalized (zoomParameter->toNormalized (2));
		zoomParameter->addDependent (this);
		uiParameters.addParameter (zoomParameter);
	}
	return res;
}

//------------------------------------------------------------------------
IPlugView* PLUGIN_API ZoomTestController::createView (FIDString name)
{
	if (strcmp (name, ViewType::kEditor) == 0)
	{
		return new VST3Editor (this, "view", "zoomtest.uidesc");
	}
	return 0;
}

//------------------------------------------------------------------------
void PLUGIN_API ZoomTestController::update (FUnknown* changedUnknown, int32 message)
{
	Parameter* param = FCast<Parameter> (changedUnknown);
	if (param && param->getInfo ().id == 1000)
	{
		size_t index = static_cast<size_t> (param->toPlain (param->getNormalized ()));
		if (index >= zoomFactors.size ())
			return;
		for (EditorVector::const_iterator it = editors.begin (), end = editors.end (); it != end; ++it)
		{
			VST3Editor* editor = dynamic_cast<VST3Editor*>(*it);
			if (editor)
				editor->setZoomFactor(zoomFactors[index].factor);
		}
	}
}

//------------------------------------------------------------------------
IController* ZoomTestController::createSubController (UTF8StringPtr name, const IUIDescription* description, VST3Editor* editor)
{
	if (UTF8StringView (name) == "ZoomController")
	{
		return new GroupController (uiParameters.getParameter (1000), this);
	}
	return 0;
}

//------------------------------------------------------------------------
void ZoomTestController::editorAttached (EditorView* editor)
{
	editors.push_back (editor);
}

//------------------------------------------------------------------------
void ZoomTestController::editorRemoved (EditorView* editor)
{
	editors.erase (std::find (editors.begin (), editors.end (), editor));
}

} // namespace VSTGUI
