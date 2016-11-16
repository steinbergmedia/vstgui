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
#include "vstgui/standalone/include/helpers/uidesc/modelbinding.h"
#include "vstgui/standalone/include/helpers/value.h"
#include "vstgui/uidescription/delegationcontroller.h"
#include "vstgui/uidescription/uiattributes.h"
#include "vstgui/uidescription/iuidescription.h"
#include "vstgui/uidescription/cstream.h"
#include "vstgui/lib/coffscreencontext.h"
#include "vstgui/lib/cframe.h"
#include "vstgui/lib/cfileselector.h"
#include "vstgui/lib/cbitmap.h"
#include "vstgui/lib/cgraphicstransform.h"
#include "vstgui/lib/platform/iplatformbitmap.h"

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

	void draw (CDrawContext* context) override
	{
		if (func)
		{
			CDrawContext::Transform t (*context, CGraphicsTransform ().translate (getViewSize ().getTopLeft()));
			func (*context, getViewSize ().getSize ());
		}
	}

	CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) override
	{
		return kMouseEventHandled;
	}

	CMouseEventResult onMouseUp (CPoint& where, const CButtonState& buttons) override
	{
		if (func && buttons.isLeftButton ())
		{
			if (buttons.getModifierState () & kShift)
				saveAsPNG ();
			else
				invalid ();
		}
		return kMouseEventHandled;
	}
	
private:
	std::string getSavePath ()
	{
		if (auto fs = owned (CNewFileSelector::create (getFrame (), CNewFileSelector::kSelectSaveFile)))
		{
			fs->setDefaultExtension (CFileExtension ("PNG", "png"));
			if (fs->runModal ())
			{
				return fs->getSelectedFile (0);
			}
		}
		return {};
	}

	void saveAsPNG ()
	{
		auto filePath = getSavePath ();
		if (filePath.empty ())
			return;

		auto size = getViewSize ().getSize ();
		auto scaleFactor = getFrame ()->getScaleFactor ();
		if (auto offscreen = COffscreenContext::create (getFrame (), size.x, size.y, scaleFactor))
		{
			offscreen->beginDraw ();
			func (*offscreen, size);
			offscreen->endDraw ();
			auto bitmap = offscreen->getBitmap ();
			if (auto platformBitmap = bitmap->getPlatformBitmap ())
			{
				auto buffer = IPlatformBitmap::createMemoryPNGRepresentation (platformBitmap);
				if (buffer.empty ())
					return;
				CFileStream stream;
				if (!stream.open (filePath.data (), CFileStream::kBinaryMode | CFileStream::kWriteMode | CFileStream::kTruncateMode))
					return;
				stream.writeRaw (buffer.data (), buffer.size ());
			}
		}
	}

	DrawFunction func;
};

void drawRects (CDrawContext& context, CPoint size)
{
	context.setDrawMode (kAliasing);
	context.setFillColor (MakeCColor (0, 0, 0, 100));
	context.drawRect (CRect ().setSize (size), kDrawFilled);

	//--
	context.setDrawMode (kAliasing);
	context.setFrameColor (kBlackCColor);
	context.setLineStyle (kLineSolid);
	context.setLineWidth (1);
	context.drawRect (CRect (5, 5, 10, 10), kDrawStroked);
	context.setFrameColor (MakeCColor (0, 0, 0, 150));
	context.drawLine ({10, 5}, {25, 5});
	context.drawLine ({5, 10}, {5, 25});
	//--
	context.setDrawMode (kAliasing);
	auto hairline = context.getHairlineSize ();
	CRect r2 (40, 5, 50, 15);
	context.setLineWidth (5);
	context.setFrameColor (CColor (0, 0, 255, 100));
	context.drawRect (r2);
	context.setLineWidth (3);
	context.setFrameColor (CColor (255, 0, 0, 100));
	context.drawRect (r2);
	context.setLineWidth (1);
	context.setFrameColor (CColor (0, 255, 0, 100));
	context.drawRect (r2);
	//--
	r2 = {60, 5, 70, 15};
	context.setLineWidth (hairline);
	context.setFrameColor (CColor (0, 255, 0, 200));
	context.drawRect (r2);
	r2.inset (hairline, hairline);
	context.setFrameColor (CColor (255, 0, 0, 200));
	context.drawRect (r2);
	r2.inset (hairline, hairline);
	context.setFrameColor (CColor (0, 0, 255, 200));
	context.drawRect (r2);
	//--
	CRect r {5, 50, 0, 0};
	r.setSize ({20, 20});
	context.setLineWidth (1);
	context.setDrawMode (kAliasing);
	context.setFillColor (MakeCColor (255, 0, 0, 255));
	context.drawRect (r, kDrawFilled);
	context.setFillColor (MakeCColor (0, 0, 0, 200));
	context.drawEllipse (r, kDrawFilled);
	context.setFrameColor (CColor (0, 255, 0, 100));
	context.drawRect (r, kDrawStroked);
	//--
	r = {50, 50, 80, 80};
	context.setDrawMode (kAliasing);
	context.setLineWidth (1);
	context.setFrameColor (CColor (0, 255, 255, 100));
	context.drawLine (r.getTopLeft (), r.getTopRight ());
	context.drawLine (r.getBottomLeft (), r.getBottomRight ());
	r.inset (10, -10);
	context.drawLine (r.getTopRight (), r.getBottomRight ());
	context.drawLine (r.getTopLeft (), r.getBottomLeft ());
	r.left++;
	r.top += 11;
	r.bottom -= 10;
	context.setFillColor (CColor (255, 0, 0, 100));
	context.drawRect (r, kDrawFilled);
	//--
	r = {90, 90, 120, 120};
	context.setDrawMode (kAliasing);
	context.setLineWidth (1);
	context.setFrameColor (CColor (0, 0, 255, 100));
	context.setFillColor (CColor (255, 0, 0, 100));
	context.drawEllipse (r, kDrawFilledAndStroked);
	r.offset (40, 0);
	context.setDrawMode (kAntiAliasing);
	context.drawEllipse (r, kDrawFilledAndStroked);
	//--
	r = {90, 130, 120, 160};
	context.setDrawMode (kAliasing);
	context.setLineWidth (1);
	context.setFrameColor (CColor (0, 0, 255, 100));
	context.setFillColor (CColor (255, 0, 0, 100));
	context.drawEllipse (r, kDrawFilled);
	r2 = r;
	r2.extend (1, 1);
	context.drawEllipse (r2, kDrawStroked);
	r.offset (40, 0);
	context.setDrawMode (kAntiAliasing);
	context.drawEllipse (r, kDrawFilled);
	r2 = r;
	r2.extend (1, 1);
	context.drawEllipse (r2, kDrawStroked);
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
