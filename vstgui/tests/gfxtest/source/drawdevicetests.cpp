//
//  drawdevicetests.cpp
//  vstgui
//
//  Created by Arne Scheffler on 26/10/2016.
//
//

#include "drawdevicetests.h"
#include "vstgui/standalone/include/iuidescwindow.h"
#include "vstgui/standalone/include/iapplication.h"
#include "vstgui/standalone/include/helpers/modelbinding.h"
#include "vstgui/standalone/include/helpers/value.h"
#include "vstgui/uidescription/delegationcontroller.h"
#include "vstgui/uidescription/uiattributes.h"
#include "vstgui/uidescription/iuidescription.h"
#include "vstgui/lib/cdrawcontext.h"
#include "vstgui/lib/cgraphicstransform.h"

namespace VSTGUI {
namespace Standalone {

class CustomDrawView : public CView
{
public:
	using DrawFunction = std::function<void (CDrawContext& context, CPoint viewSize)>;
	
	CustomDrawView (DrawFunction func)
	: CView (CRect (0, 0, 0, 0))
	, func (func)
	{
	}

	void draw (CDrawContext* context)
	{
		if (func)
		{
			CDrawContext::Transform t (*context, CGraphicsTransform ().translate (getViewSize ().getTopLeft()));
			func (*context, getViewSize ().getSize ());
		}
	}

private:
	DrawFunction func;
};

void drawRects (CDrawContext& context, CPoint size)
{
	context.setDrawMode (kAliasing);
	context.setFillColor (MakeCColor (0, 0, 0, 100));
	context.drawRect (CRect ().setSize (size), kDrawFilled);

	context.setDrawMode (kAntiAliasing|kNonIntegralMode);
	context.setFrameColor (kBlackCColor);
	context.setLineStyle (kLineSolid);
	context.setLineWidth (1);
	context.drawRect (CRect (5, 5, 15, 15), kDrawStroked);
	context.drawLine ({15, 5}, {25, 5});
	context.drawLine ({5, 15}, {5, 25});
}

class ViewCreator : public DelegationController
{
public:
	ViewCreator (IController* parent) : DelegationController (parent) {}

	CView* createView (const UIAttributes& attributes, const IUIDescription* description)
	{
		if (auto customViewName = attributes.getAttributeValue (IUIDescription::kCustomViewName))
		{
			if (*customViewName == "RectsView")
			{
				return new CustomDrawView ([] (auto& ctx, auto size) { drawRects (ctx, size); });
			}
			else if (*customViewName == "PathsView")
			{
				
			}
		}
		return DelegationController::createView (attributes, description);
	}

private:

};

class DrawDeviceTestsCustomization : public UIDesc::ICustomization
{
public:
	DrawDeviceTestsCustomization ()
	{
		modelBindings = UIDesc::ModelBindingCallbacks::make ();
		modelBindings->addValue (Value::makeStringListValue ("ViewSelector", {"Lines/Rects", "Paths"}));
	}
	
	IController* createController (const UTF8StringView& name, IController* parent,
								   const IUIDescription* uiDesc) override
   {
	   if (name == "ViewCreator")
		   return new ViewCreator (parent);
	   return nullptr;
   }

	UIDesc::ModelBindingPtr getModelBinding () const { return modelBindings; }
private:
	UIDesc::ModelBindingCallbacksPtr modelBindings;
};

void makeDrawDeviceTestsWindow ()
{
	static UTF8String windowTitle = "DrawDeviceTests";
	const auto& windows = IApplication::instance().getWindows ();
	for (auto& window : windows)
	{
		if (window->getTitle () == windowTitle)
		{
			window->activate ();
			return;
		}
	}

	auto drawDeviceTestsCustomization = std::make_shared<DrawDeviceTestsCustomization> ();

	UIDesc::Config config;
	config.uiDescFileName = "DrawDeviceTests.uidesc";
	config.viewName = "Window";
	config.customization = drawDeviceTestsCustomization;
	config.modelBinding = drawDeviceTestsCustomization->getModelBinding ();
	config.windowConfig.title = windowTitle;
	config.windowConfig.autoSaveFrameName = "DrawDeviceTestsWindow";
	config.windowConfig.style.border ().close ().size ().centered ();
	if (auto window = UIDesc::makeWindow (config))
		window->show ();
}

} // Standalone
} // VSTGUI
