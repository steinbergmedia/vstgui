//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 4.3
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2015, Steinberg Media Technologies, All Rights Reserved
//-----------------------------------------------------------------------------
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
// 
//   * Redistributions of source code must retain the above copyright notice, 
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation 
//     and/or other materials provided with the distribution.
//   * Neither the name of the Steinberg Media Technologies nor the names of its
//     contributors may be used to endorse or promote products derived from this 
//     software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#include "uidescription test.h"
#include "vstgui/uidescription/delegationcontroller.h"
#include "pluginterfaces/base/ibstream.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include <cmath>
#include <cassert>

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
	PeakParameter (int32 flags, ParamID id, const TChar* title);

	void toString (ParamValue normValue, String128 string) const VSTGUI_OVERRIDE_VMETHOD;
	bool fromString (const TChar* string, ParamValue& normValue) const VSTGUI_OVERRIDE_VMETHOD;
};

//------------------------------------------------------------------------
PeakParameter::PeakParameter (int32 flags, ParamID id, const TChar* title)
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
	Steinberg::String str;
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
bool UIDescriptionBaseController::isPrivateParameter (const Steinberg::Vst::ParamID paramID)
{
	return uiParameters.getParameter (paramID) != 0 ? true : false;
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
class RemoveModalViewAnimation : public Animation::AlphaValueAnimation
{
public:
	RemoveModalViewAnimation (float endValue, bool forceEndValueOnFinish = false) : Animation::AlphaValueAnimation (endValue, forceEndValueOnFinish) {}

	void animationFinished (CView* view, IdStringPtr name, bool wasCanceled) VSTGUI_OVERRIDE_VMETHOD
	{
		Animation::AlphaValueAnimation::animationFinished (view, name, wasCanceled);
		if (view == view->getFrame ()->getModalView ())
		{
			view->getFrame ()->setModalView (0);
		}
	}

};

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
class ModalViewController : public DelegationController
{
public:
	ModalViewController (IController* controller, const UIDescription* desc) : DelegationController (controller), desc (desc) {}

	IControlListener* getControlListener (const char* controlTagName) VSTGUI_OVERRIDE_VMETHOD { return this; }
	void valueChanged (CControl* pControl) VSTGUI_OVERRIDE_VMETHOD
	{
		if (pControl->getValue () > 0.)
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
						frame->getTransform ().inverse ().transform (center);
						CRect viewSize = view->getViewSize ();
						viewSize.offset (center.x - viewSize.getWidth () / 2, center.y - viewSize.getHeight () / 2);
						view->setViewSize (viewSize);
						view->setMouseableArea (viewSize);
						frame->setModalView (view);
						view->setAlphaValue (0.f);
						view->addAnimation ("AlphaAnimation", new Animation::AlphaValueAnimation (1.f), new Animation::PowerTimingFunction (240, 2));
						pControl->setValue (0);
						view->forget ();
					}
					break;
				}
				case 1:
				{
					CView* modalView = pControl->getFrame ()->getModalView ();
					modalView->addAnimation ("AlphaAnimation", new RemoveModalViewAnimation (0.f), new Animation::PowerTimingFunction (240, 0.5));
					pControl->setMouseEnabled (false);
					break;
				}
			}
		}
	}

protected:
	const UIDescription* desc;
};

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
class SplitViewController : public DelegationController, public ISplitViewController, public ISplitViewSeparatorDrawer, public CBaseObject
{
public:
	SplitViewController (IController* controller)
	: DelegationController (controller)
	, path (0)
	, gradient (0)
	{
		for (int32_t i = 0; i < 3; i++)
			viewSizes[i] = -1.;
	}

	~SplitViewController ()
	{
		if (gradient)
			gradient->forget ();
		if (path)
			path->forget ();
	}
	
	bool getSplitViewSizeConstraint (int32_t index, CCoord& minWidth, CCoord& maxWidth, CSplitView* splitView) VSTGUI_OVERRIDE_VMETHOD
	{
		if (index == 0)
		{
			minWidth = 30;
		}
		else if (index == 1)
		{
			minWidth = 50;
		}
		else if (index == 2)
		{
			minWidth = 100;
			maxWidth = 300;
		}
		return true;
	}
	
	ISplitViewSeparatorDrawer* getSplitViewSeparatorDrawer (CSplitView* splitView) VSTGUI_OVERRIDE_VMETHOD
	{
		return this;
	}

	bool storeViewSize (int32_t index, const CCoord& size, CSplitView* splitView) VSTGUI_OVERRIDE_VMETHOD
	{
		if (index < 3)
		{
			viewSizes[index] = size;
			return true;
		}
		return false;
	}
	
	bool restoreViewSize (int32_t index, CCoord& size, CSplitView* splitView) VSTGUI_OVERRIDE_VMETHOD
	{
		if (index < 3 && viewSizes[index] != -1.)
		{
			size = viewSizes[index];
			return true;
		}
		return false;
	}

	void drawSplitViewSeparator (CDrawContext* context, const CRect& size, int32_t flags, int32_t index, CSplitView* splitView) VSTGUI_OVERRIDE_VMETHOD
	{
		if (path == 0)
		{
			path = context->createGraphicsPath ();
			path->addRect (CRect (0., 0., 1., 1.));
		}
		if (path && gradient == 0)
		{
			CColor c1 (140, 140, 140, 255);
			CColor c2 (100, 100, 100, 255);
			gradient = path->createGradient (0., 1., c1, c2);
		}
		if (path && gradient)
		{
			CGraphicsTransform tm;
			tm.translate (size.left, size.top + 1);
			tm.scale (size.getWidth (), size.getHeight () - 2);
			CPoint start;
			CPoint end;
			if (index == 0)
			{
				start = CPoint (size.left, size.top);
				end = CPoint (size.right, size.top);
			}
			else
			{
				start = CPoint (size.right, size.top);
				end = CPoint (size.left, size.top);
			}
			context->saveGlobalState ();
			context->setGlobalAlpha (context->getGlobalAlpha () * (flags == ISplitViewSeparatorDrawer::kMouseOver ? 0.9f : 0.6f));
			context->fillLinearGradient (path, *gradient, start, end, false, &tm);
			context->setFillColor (MakeCColor (255, 0, 0, 155));
			context->drawGraphicsPath (path, CDrawContext::kPathFilled, &tm);
			context->restoreGlobalState ();
		}
	}

	CLASS_METHODS(SplitViewController, CBaseObject)
protected:
	CCoord viewSizes[3];
	CGraphicsPath* path;
	CGradient* gradient;
};

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
class LineStyleTestView : public CControl
{
public:
	LineStyleTestView (const CRect& size) : CControl (size) {}

	void setupLineStyle (CDrawContext* context)
	{
		context->setFrameColor (kBlackCColor);
		context->setDrawMode (kAntiAliasing);
		context->setLineWidth (5);
		const CCoord kDefaultOnOffDashLength[] = {1, 2};
		switch ((int32_t)value)
		{
			case 0: context->setLineStyle (kLineSolid); break;
			case 1: context->setLineStyle (CLineStyle (CLineStyle::kLineCapButt, CLineStyle::kLineJoinRound)); break;
			case 2: context->setLineStyle (CLineStyle (CLineStyle::kLineCapButt, CLineStyle::kLineJoinBevel)); break;
			case 3: context->setLineStyle (kLineOnOffDash); break;
			case 4: context->setLineStyle (CLineStyle (CLineStyle::kLineCapRound, CLineStyle::kLineJoinMiter, 0, 2, kDefaultOnOffDashLength)); break;
			case 5: context->setLineStyle (CLineStyle (CLineStyle::kLineCapSquare, CLineStyle::kLineJoinMiter, 0, 2, kDefaultOnOffDashLength)); break;
		}
		
	}

	void draw (CDrawContext* context) VSTGUI_OVERRIDE_VMETHOD
	{
		CGraphicsPath* path = context->createGraphicsPath ();
		if (path)
		{
			CRect r (getViewSize ());
			r.inset (5, 5);
			path->beginSubpath (CPoint (r.left + r.getWidth () / 2, r.top));
			path->addLine (CPoint (r.left, r.bottom));
			path->addLine (CPoint (r.right, r.bottom));
			path->closeSubpath ();
			setupLineStyle (context);
			context->drawGraphicsPath (path, CDrawContext::kPathStroked);
			path->forget ();
		}
		setDirty (false);
	}

	CLASS_METHODS(LineStyleTestView, CControl)
};

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
class DrawPrimitivesTestView : public CView
{
public:
	DrawPrimitivesTestView (const CRect& size) : CView (size) {}
	
	void draw (CDrawContext* context) VSTGUI_OVERRIDE_VMETHOD
	{
		context->setDrawMode (kAliasing);
		context->setLineWidth (1);
		context->setLineStyle (kLineSolid);

		context->setFrameColor (kBlackCColor);

		context->setLineWidth (1);
		context->setLineStyle (kLineSolid);
		context->setFrameColor (kBlackCColor);

		CDrawContext::LineList lines;
		const CCoord gridOffset = 10;
		CPoint p = getViewSize ().getTopLeft ();
		while (p.x < getWidth ())
		{
			CPoint p2 (p);
			p2.y = getHeight ();
			lines.push_back (std::make_pair (p, p2));
			p.offset (gridOffset, 0);
		}
		p = getViewSize ().getTopLeft ();
		while (p.y < getHeight ())
		{
			CPoint p2 (p);
			p2.x = getWidth ();
			lines.push_back (std::make_pair (p, p2));
			p.offset (0, gridOffset);
		}
		context->drawLines (lines);

		context->setLineWidth (1);
		context->setDrawMode (kAliasing);

		CColor c (kGreenCColor);
		context->setFrameColor (c);
		CRect r (getViewSize());
		r.inset (1, 1);
		context->drawRect (r);
		context->drawLine (r.getTopLeft (), r.getTopRight ());
		context->drawLine (r.getTopRight (), r.getBottomRight ());
		context->drawLine (r.getBottomRight (), r.getBottomLeft ());
		context->drawLine (r.getBottomLeft (), r.getTopLeft ());

		SharedPointer<CGraphicsPath> path = owned (context->createGraphicsPath ());
		r.inset (4, 4);
		path->addRect (r);
		context->drawGraphicsPath (path, CDrawContext::kPathStroked);
		path = owned (context->createGraphicsPath ());
		path->beginSubpath (r.getTopLeft ());
		path->addLine (r.getTopRight ());
		path->addLine (r.getBottomRight ());
		path->addLine (r.getBottomLeft ());
		path->addLine (r.getTopLeft ());
		context->drawGraphicsPath (path, CDrawContext::kPathStroked);
		

		context->setDrawMode (kAliasing);
		context->setFillColor (c);
		r = getViewSize ();
		r.offset (gridOffset + 1, gridOffset + 1);
		r.setWidth (gridOffset - 1);
		r.setHeight (gridOffset - 1);
		context->drawRect (r, kDrawFilled);
		r.offset (gridOffset * 2, gridOffset * 2);
		context->setDrawMode (kAntiAliasing);
		context->drawRect (r, kDrawFilled);
		
		context->setDrawMode (kAntiAliasing|kNonIntegralMode);
		c = kRedCColor;
		c.alpha = 150;
		context->setFillColor (c);
		r = getViewSize ();
		r.offset (51.5, 51.5);
		r.setWidth (gridOffset-1.5);
		r.setHeight (gridOffset-1.5);
		context->drawRect (r, kDrawFilled);

		r.offset (0, r.getHeight ()+1);
		context->setDrawMode (kAntiAliasing);
		context->drawRect (r, kDrawFilled);

		r = getViewSize ();
		r.setWidth (gridOffset * 4. + 1);
		r.setHeight (gridOffset * 4. + 1);
		r.offset (gridOffset*8, gridOffset*4);
		context->setDrawMode (kAliasing);
		context->setFrameColor (kBlueCColor);
		context->drawRect (r, kDrawStroked);
		context->setFrameColor (kRedCColor);
		context->drawEllipse (r, kDrawStroked);
		r.offset (r.getWidth () + gridOffset - 1, 0);
		context->setFrameColor (kBlueCColor);
		context->drawRect (r, kDrawStroked);
		context->setFillColor (kRedCColor);
		context->drawEllipse (r, kDrawFilled);

		context->setFillColor (kRedCColor);
		context->setFrameColor (kBlackCColor);
		context->setLineWidth (2.);
		CFontDesc font (*kSystemFont);
		font.setSize (50);
		path = owned (context->createTextPath (&font, "Test"));
		{
			CDrawContext::Transform t (*context, CGraphicsTransform ().translate (20., 90.));
			context->setDrawMode (kAntiAliasing|kNonIntegralMode);
			context->drawGraphicsPath (path, CDrawContext::kPathStroked);
			context->drawGraphicsPath (path);
		}
		{
			CDrawContext::Transform t (*context, CGraphicsTransform ().translate (20. + std::ceil (path->getBoundingBox ().getWidth () / 10.) * 10, 90.));
			context->setDrawMode (kAntiAliasing);
			context->drawGraphicsPath (path, CDrawContext::kPathStroked);
			context->drawGraphicsPath (path);
		}

		path = owned (context->createGraphicsPath ());
		CPoint pathOffset (20., 190.);
		CRect pathSize (0., 0., 100., 20.);
		path->addRect (pathSize);
		context->setFrameColor (kRedCColor);
		context->setLineWidth (1.);
		context->setDrawMode (kAntiAliasing);
		CRect pathBounds = path->getBoundingBox ();

		CDrawContext::Transform gt (*context, CGraphicsTransform ().translate (pathOffset.x, pathOffset.y));

		const CPoint center = pathBounds.getCenter ();
		context->drawPoint (center, kGreenCColor);
		for (double r = 0.; r < 180.; r += 30.)
		{
			CGraphicsTransform t;
			t.rotate (r, center);
			context->drawGraphicsPath (path, CDrawContext::kPathStroked, &t);
		}

		context->setDrawMode (kAntiAliasing|kNonIntegralMode);
		{
			CDrawContext::Transform t (*context, CGraphicsTransform ().translate (110., 0.));
			for (double r = 0.; r < 180.; r += 30.)
			{
				CGraphicsTransform transform;
				transform.rotate (r, center);
				context->drawGraphicsPath (path, CDrawContext::kPathStroked, &transform);
			}
		}

		context->setDrawMode (kAntiAliasing);
		SharedPointer<CGradient> gradient = CGradient::create (0, 1, MakeCColor (255, 0, 0, 255), MakeCColor (0, 0, 255, 100));
		{
			CDrawContext::Transform t (*context, CGraphicsTransform ().translate (220., 0.));
			CPoint start = pathBounds.getTopLeft ();
			CPoint end = pathBounds.getBottomRight ();
			for (double r = 0.; r < 180.; r += 30.)
			{
				CGraphicsTransform transform;
				transform.rotate (r, center);
				context->fillLinearGradient (path, *gradient, start, end, false, &transform);
			}
		}

		context->setDrawMode (kAntiAliasing|kNonIntegralMode);
		{
			CDrawContext::Transform t (*context, CGraphicsTransform ().translate (330., 0.));
			for (double r = 0.; r < 180.; r += 10.)
			{
				CGraphicsTransform transform;
				transform.rotate (r, center);
				CPoint start = pathBounds.getTopLeft ();
				start.y += pathBounds.getHeight() / 2.;
				CPoint end = pathBounds.getBottomRight ();
				end.y -= pathBounds.getHeight() / 2.;
				transform.transform (start);
				transform.transform (end);
				context->fillLinearGradient (path, *gradient, start, end, false, &transform);
//				context->drawLine (start, end);
			}
		}
	}
};

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
UIDescriptionTestController::UIDescriptionTestController ()
: splitViewController (0)
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
		slp->appendString (USTRING("Controls"));
		slp->appendString (USTRING("Miscellaneous"));
		slp->appendString (USTRING("ScrollView"));
		slp->appendString (USTRING("SplitView"));
		slp->appendString (USTRING("Fonts/LineStyle"));
		slp->appendString (USTRING("Primitives"));
		uiParameters.addParameter (slp);
		
		slp = new StringListParameter (USTRING("LineStyle"), 20001);
		slp->appendString (USTRING("Solid"));
		slp->appendString (USTRING("Solid Round Join"));
		slp->appendString (USTRING("Solid Bevel Join"));
		slp->appendString (USTRING("On-Off"));
		slp->appendString (USTRING("On-Off Round Cap"));
		slp->appendString (USTRING("On-Off Square Cap"));
		uiParameters.addParameter (slp);
		
		slp = new StringListParameter (USTRING("KnobStepCountTestParameter"), 20002);
		slp->appendString (USTRING("1"));
		slp->appendString (USTRING("2"));
		slp->appendString (USTRING("3"));
		slp->appendString (USTRING("4"));
		slp->appendString (USTRING("5"));
		slp->appendString (USTRING("6"));
		uiParameters.addParameter (slp);
	}
	return res;
}

//------------------------------------------------------------------------
IPlugView* PLUGIN_API UIDescriptionTestController::createView (FIDString name)
{
	if (strcmp (name, ViewType::kEditor) == 0)
	{
		VST3Editor* editor = new VST3Editor (this, "view", "myEditor.uidesc");
		std::vector<double> zoomFactors;
		zoomFactors.push_back (0.75);
		zoomFactors.push_back (1.);
		zoomFactors.push_back (1.25);
		zoomFactors.push_back (1.5);
		zoomFactors.push_back (1.75);
		zoomFactors.push_back (2.);
		editor->setAllowedZoomFactors (zoomFactors);
		return editor;
	}
	return 0;
}

//------------------------------------------------------------------------
class UIDescriptionTestControllerMenuHandler : public CBaseObject
{
public:
	UIDescriptionTestControllerMenuHandler (Parameter* param) : param (param) {}

	CMessageResult notify (CBaseObject* sender, IdStringPtr message) VSTGUI_OVERRIDE_VMETHOD
	{
		if (message == CCommandMenuItem::kMsgMenuItemSelected)
		{
			CCommandMenuItem* menuItem = dynamic_cast<CCommandMenuItem*>(sender);
			if (menuItem)
			{
				param->setNormalized (param->toNormalized (menuItem->getTag ()));
				return kMessageNotified;
			}
		}
		return kMessageUnknown;
	}
	
protected:
	IPtr<Parameter> param;
};

//------------------------------------------------------------------------
COptionMenu* UIDescriptionTestController::createContextMenu (const CPoint& pos, VST3Editor* editor)
{
	Parameter* tabParameter = getParameterObject (20000);
	if (tabParameter)
	{
		UIDescriptionTestControllerMenuHandler* menuHandler = new UIDescriptionTestControllerMenuHandler (tabParameter);
		COptionMenu* menu = new COptionMenu ();
		menu->setStyle (kMultipleCheckStyle);
		for (int32 i = 0; i <= tabParameter->getInfo ().stepCount; i++)
		{
			String128 valueString;
			tabParameter->toString (tabParameter->toNormalized (i), valueString);
			Steinberg::String str (valueString);
			str.toMultiByte (kCP_Utf8);
			CMenuItem* item = menu->addEntry (new CCommandMenuItem (str.text8 (), i, menuHandler));
			if (tabParameter->getNormalized () == tabParameter->toNormalized (i))
				item->setChecked (true);
		}
		COptionMenu* mainMenu = new COptionMenu ();
		mainMenu->addEntry (new CMenuItem ("Show Tab"))->setSubmenu (menu);
		menuHandler->forget ();
		return mainMenu;
	}
	return 0;
}

//------------------------------------------------------------------------
IController* UIDescriptionTestController::createSubController (const char* name, const IUIDescription* description, VST3Editor* editor)
{
	if (strcmp (name, "ModalViewController") == 0)
	{
		return new ModalViewController (editor, dynamic_cast<const UIDescription*> (description));
	}
	if (strcmp (name, "SplitViewController") == 0)
	{
		if (splitViewController == 0)
			splitViewController = new SplitViewController (editor);
		splitViewController->remember ();
		return dynamic_cast<IController*> (splitViewController);
	}
	return 0;
}

//------------------------------------------------------------------------
CView* UIDescriptionTestController::createCustomView (UTF8StringPtr name, const UIAttributes& attributes, const IUIDescription* description, VST3Editor* editor)
{
	UTF8StringView viewName (name);
	if (viewName == "LineStyleTestView")
	{
		return new LineStyleTestView (CRect (0, 0, 0, 0));
	}
	else if (viewName == "DrawPrimitivesTestView")
	{
		return new DrawPrimitivesTestView (CRect (0, 0, 0, 0));
	}
	return 0;
}

//------------------------------------------------------------------------
CView* UIDescriptionTestController::verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description, VST3Editor* editor)
{
	CSegmentButton* button = dynamic_cast<CSegmentButton*>(view);
	if (button && button->getTag () == 20000)
	{
		StringListParameter* slp = dynamic_cast<StringListParameter*> (getParameterObject (20000));
		vstgui_assert (slp && button->getSegments ().size () == static_cast<size_t> (slp->getInfo ().stepCount + 1));
		for (uint32_t i = 0; i <= static_cast<uint32_t> (slp->getInfo ().stepCount); i++)
		{
			String128 str = {};
			slp->toString (slp->toNormalized (i), str);
			Steinberg::String str2 (str);
			str2.toMultiByte (kCP_Utf8);
			button->getSegments ()[i].name = str2.text8 ();
		}
	}
	return view;
}

//------------------------------------------------------------------------
void UIDescriptionTestController::willClose (VST3Editor* editor)
{
	if (splitViewController)
	{
		splitViewController->forget ();
		splitViewController = 0;
	}
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
UIDescriptionTestProcessor::UIDescriptionTestProcessor ()
: peak (0.f)
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
tresult PLUGIN_API UIDescriptionTestProcessor::setProcessing (Steinberg::TBool state)
{
	peak = 0.f;
	return AudioEffect::setProcessing (state);
}

//------------------------------------------------------------------------
tresult PLUGIN_API UIDescriptionTestProcessor::process (ProcessData& data)
{
	ParamValue localPeak = 0.;
	for (int32 sample = 0; sample < data.numSamples; sample++)
	{
		for (int32 channel = 0; channel < data.inputs[0].numChannels; channel++)
		{
			float value = data.inputs[0].channelBuffers32[channel][sample];
			data.outputs[0].channelBuffers32[channel][sample] = value;
			value = std::abs (value);
			if (value > localPeak)
				localPeak = value;
		}
	}
	if (data.outputParameterChanges)
	{
		if (localPeak < peak)
		{
			float vuDecrease = static_cast<float> (data.numSamples > 0 ? data.numSamples / processSetup.sampleRate : 0.001);
			peak -= vuDecrease;
			if (peak < 0.f)
				peak = 0.f;
		}
		else
			peak = static_cast<float> (localPeak);
		int32 index;
		IParamValueQueue* queue = data.outputParameterChanges->addParameterData (kPeakParam, index);
		if (queue)
			queue->addPoint (0, peak, index);
	}
	return kResultTrue;
}

} // namespace VSTGUI

