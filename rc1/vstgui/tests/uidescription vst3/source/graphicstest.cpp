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

#include "graphicstest.h"
#include "vstgui/lib/animation/animations.h"
#include "vstgui/uidescription/delegationcontroller.h"
#include "bitmapsaturator.h"

using namespace Steinberg;
using namespace Steinberg::Vst;
using namespace VSTGUI::Animation;

namespace VSTGUI {

//------------------------------------------------------------------------
class GraphicsView : public CView, public IAnimationTarget
{
public:
	GraphicsView ();
	~GraphicsView ();
	
	void setRotation (float angle) { pathRotation = angle; invalid (); }
	void setFillGradient (bool state) { fillGradient = state; invalid (); }
	void setStrokePath (bool state) { strokePath = state; invalid (); }
	void setPathOne (int32_t index);
	void setPathTwo (int32_t index);
	
	void draw (CDrawContext *pContext) VSTGUI_OVERRIDE_VMETHOD;

	bool attached (CView* parent) VSTGUI_OVERRIDE_VMETHOD;
	bool removed (CView* parent) VSTGUI_OVERRIDE_VMETHOD;

	// IAnimationTarget
	void animationStart (CView* view, const char* name) VSTGUI_OVERRIDE_VMETHOD;
	void animationTick (CView* view, const char* name, float pos) VSTGUI_OVERRIDE_VMETHOD;
	void animationFinished (CView* view, const char* name, bool wasCanceled) VSTGUI_OVERRIDE_VMETHOD;
	
	enum PathType {
		kType1,
		kType2,
		kType3,
		kNumPaths
	};

protected:
	static CGraphicsPath* buildPath (CDrawContext* context, PathType type);

	CBitmap* bitmap2;

	CGraphicsPath* path[kNumPaths];
	int32_t path1Index;
	int32_t path2Index;

	CGradient* gradient;
	float pathRotation;
	bool fillGradient;
	bool strokePath;
};

//------------------------------------------------------------------------
class FadeViewContainer : public CViewContainer
{
public:
	FadeViewContainer () : CViewContainer (CRect (0, 0, 0, 0)) { setAlphaValue (0.3f); }
	
	CMouseEventResult onMouseEntered (CPoint &where, const CButtonState& buttons) VSTGUI_OVERRIDE_VMETHOD
	{
		addAnimation ("AlphaAnimation", new Animation::AlphaValueAnimation (1.f), new Animation::LinearTimingFunction (100));
		return kMouseEventHandled;
	}

	CMouseEventResult onMouseExited (CPoint &where, const CButtonState& buttons) VSTGUI_OVERRIDE_VMETHOD
	{
		addAnimation ("AlphaAnimation", new Animation::AlphaValueAnimation (0.3f), new Animation::LinearTimingFunction (100));
		return kMouseEventHandled;
	}
};

//------------------------------------------------------------------------
class GraphicsViewController : public DelegationController, public CBaseObject
{
public:
	GraphicsViewController (IController* controller) : DelegationController (controller), graphicsView (0) {}
	~GraphicsViewController ();

	CView* verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description) VSTGUI_OVERRIDE_VMETHOD;
	IControlListener* getControlListener (const char* controlTagName) VSTGUI_OVERRIDE_VMETHOD;
	void valueChanged (CControl* pControl) VSTGUI_OVERRIDE_VMETHOD;

	CMessageResult notify (CBaseObject* object, IdStringPtr message) VSTGUI_OVERRIDE_VMETHOD;
protected:
	GraphicsView* graphicsView;
	std::list<CControl*> controls;
};

//------------------------------------------------------------------------
FUID GraphicsTestProcessor::cid (0xFFEF9061, 0x21994D6F, 0x91F5F358, 0x6AB1FA41);
FUID GraphicsTestController::cid (0xDF130F0E, 0xFD0A4543, 0x9AEE67FB, 0xD868F0AA);

//------------------------------------------------------------------------
tresult PLUGIN_API GraphicsTestController::initialize (FUnknown* context)
{
	tresult res = UIDescriptionBaseController::initialize (context);
	if (res == kResultTrue)
	{
		// add ui parameters
		StringListParameter* slp = new StringListParameter (USTRING("GraphicsPath1"), 20000);
		slp->appendString (USTRING("Type 1"));
		slp->appendString (USTRING("Type 2"));
		slp->appendString (USTRING("Type 3"));
		uiParameters.addParameter (slp);
		slp = new StringListParameter (USTRING("GraphicsPath2"), 20001);
		slp->appendString (USTRING("Type 1"));
		slp->appendString (USTRING("Type 2"));
		slp->appendString (USTRING("Type 3"));
		uiParameters.addParameter (slp);
	}
	return res;
}

//------------------------------------------------------------------------
IPlugView* PLUGIN_API GraphicsTestController::createView (FIDString name)
{
	if (strcmp (name, ViewType::kEditor) == 0)
	{
		VST3Editor* editor = new VST3Editor (this, "view", "GraphicsTest.uidesc");
		std::vector<double> zoomFactors;
		zoomFactors.push_back (0.75);
		zoomFactors.push_back (1.);
		zoomFactors.push_back (1.5);
		zoomFactors.push_back (2.);
		editor->setAllowedZoomFactors (zoomFactors);
		return editor;
	}
	return 0;
}

//------------------------------------------------------------------------
CView* GraphicsTestController::createCustomView (const char* name, const UIAttributes& attributes, const IUIDescription* description, VST3Editor* editor)
{
	if (strcmp (name, "GraphicsView") == 0)
	{
		return new GraphicsView;
	}
	else if (strcmp (name, "FadeViewContainer") == 0)
	{
		return new FadeViewContainer;
	}
	return 0;
}

//------------------------------------------------------------------------
IController* GraphicsTestController::createSubController (const char* name, const IUIDescription* description, VST3Editor* editor)
{
	if (strcmp (name, "GraphicsViewController") == 0)
		return new GraphicsViewController (editor);
	return 0;
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
GraphicsViewController::~GraphicsViewController ()
{
	for (std::list<CControl*>::const_iterator it = controls.begin (); it != controls.end (); it++)
		(*it)->removeDependency (this);
}

//------------------------------------------------------------------------
CView* GraphicsViewController::verifyView (CView* view, const UIAttributes& attributes, const IUIDescription* description)
{
	if (graphicsView == 0)
		graphicsView = dynamic_cast<GraphicsView*> (view);
	CControl* control = dynamic_cast<CControl*> (view);
	if (control)
	{
		if (control->getTag () == 20000 || control->getTag () == 20001)
		{
			control->addDependency (this);
			controls.push_back (control);
			valueChanged (control);
		}
	}
	return DelegationController::verifyView (view, attributes, description);
}

//------------------------------------------------------------------------
IControlListener* GraphicsViewController::getControlListener (const char* controlTagName)
{
	if (strcmp (controlTagName, "Rotate") == 0 
	 || strcmp (controlTagName, "FillGradient") == 0
	 || strcmp (controlTagName, "StrokePath") == 0
	)
	{
		return this;
	}
	return DelegationController::getControlListener (controlTagName);
}

//------------------------------------------------------------------------
CMessageResult GraphicsViewController::notify (CBaseObject* object, IdStringPtr message)
{
	if (message == CControl::kMessageValueChanged)
	{
		valueChanged (dynamic_cast<CControl*> (object));
		return kMessageNotified;
	}
	return kMessageUnknown;
}

//------------------------------------------------------------------------
void GraphicsViewController::valueChanged (CControl* pControl)
{
	if (graphicsView == 0)
		return;
	switch (pControl->getTag ())
	{
		case 0: // rotate star
		{
			if (pControl->getValue () > 0)
			{
				graphicsView->remember (); // the animator will call a forget on it
				graphicsView->addAnimation ("Rotation", graphicsView, new RepeatTimingFunction (new LinearTimingFunction (4000), -1, false));
			}
			else
			{
				graphicsView->removeAnimation ("Rotation");
			}
			break;
		}
		case 1: // fill gradient
		{
			graphicsView->setFillGradient (pControl->getValue () > 0.f);
			break;
		}
		case 2: // stroke path
		{
			graphicsView->setStrokePath (pControl->getValue () > 0.f);
			break;
		}
		case 20000:
		{
			graphicsView->setPathOne (static_cast<int32_t> (pControl->getValue ()));
			break;
		}
		case 20001:
		{
			graphicsView->setPathTwo (static_cast<int32_t> (pControl->getValue ()));
			break;
		}
	}
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
GraphicsView::GraphicsView ()
: CView (CRect (0, 0, 0, 0))
, gradient (0)
, pathRotation (0.f)
, fillGradient (false)
, strokePath (false)
, bitmap2 (0)
, path1Index (0)
, path2Index (1)
{
	for (int32_t i = 0; i < kNumPaths; i++)
		path[i] = 0;
}

//------------------------------------------------------------------------
GraphicsView::~GraphicsView ()
{
}

//------------------------------------------------------------------------
void GraphicsView::draw (CDrawContext *pContext)
{
	CView::draw (pContext);
	if (path[kType1] == 0)
		path[kType1] = buildPath (pContext, kType1);
	if (path[kType2] == 0)
		path[kType2] = buildPath (pContext, kType2);
	if (path[kType3] == 0)
		path[kType3] = buildPath (pContext, kType3);
	if (path[path1Index] == 0 || path[path2Index] == 0)
		return;
	CGraphicsPath* drawPath = pContext->createGraphicsPath ();
	if (drawPath)
	{
		drawPath->addPath (*path[path1Index]);
		drawPath->closeSubpath ();

		CRect pathBounds = CRect (0, 0, 100, 100);

		CGraphicsTransform t;
		t.rotate (pathRotation*2.f, pathBounds.getCenter ());
		drawPath->addPath (*path[path2Index], &t);

		pContext->setDrawMode (kAntiAliasing);
		pContext->setLineWidth (getWidth () / 100.);
		pContext->setFrameColor (kRedCColor);
		pContext->setFillColor (kBlueCColor);
		double lengths[] = {1, 3, 4, 2};
		CLineStyle lineStyle (CLineStyle::kLineCapSquare, CLineStyle::kLineJoinBevel, 0, 4, lengths);
		pContext->setLineStyle (lineStyle);

		double scaleX = getWidth ()/pathBounds.getWidth()/1.;
		double scaleY = getHeight ()/pathBounds.getHeight()/1.;
		CGraphicsTransform t2;
		t2.rotate (pathRotation, pathBounds.getCenter ());
		t2.scale (scaleX, scaleY);
		t2.translate (size.left + getWidth () / 2. - (pathBounds.getWidth () * scaleX) / 2., size.top + getHeight () / 2. - (pathBounds.getHeight () * scaleY) / 2.);
		if (strokePath)
			pContext->drawGraphicsPath (drawPath, CDrawContext::kPathStroked, &t2);
		if (fillGradient)
		{
			if (gradient == 0)
			{
				CColor c1 (kGreenCColor);
				CColor c2 (kBlackCColor);
				c1.alpha = 100; 
				gradient = drawPath->createGradient (0.0, 0.7, c1, c2);
			}
			if (gradient)
				pContext->fillLinearGradient (drawPath, *gradient, size.getTopLeft (), size.getBottomRight (), false, &t2);
		}
		else
		{
			pContext->drawGraphicsPath (drawPath, CDrawContext::kPathFilled, &t2);
		}
		drawPath->forget ();
	}
}

//------------------------------------------------------------------------
CGraphicsPath* GraphicsView::buildPath (CDrawContext* context, PathType type)
{
	CGraphicsPath* path = context->createGraphicsPath ();
	if (path == 0)
		return 0;
	switch (type)
	{
		case kType1:
		{
			path->beginSubpath		(50.246, 20.672);
			path->addBezierCurve	(47.266, 20.672, 44.818, 18.224, 44.818, 15.237);
			path->addBezierCurve	(44.818, 12.221, 47.266, 9.785, 50.246, 9.785);
			path->addBezierCurve	(53.263, 9.785, 55.668, 12.221, 55.668, 15.237);
			path->addBezierCurve	(55.668, 18.227, 53.263, 20.672, 50.246, 20.672);
			path->beginSubpath		(94.167, 68.284);
			path->addLine			(91.275, 50.233);
			path->addBezierCurve	(90.931, 48.539, 89.456, 47.253, 87.711, 47.253);
			path->addBezierCurve	(86.634, 47.253, 85.692, 47.715, 84.995, 48.448);
			path->addLine			(71.636, 60.961);
			path->addBezierCurve	(71.43, 61.156, 71.208, 61.378, 71.031, 61.633);
			path->addBezierCurve	(69.766, 63.348, 70.116, 65.766, 71.834, 67.043);
			path->addBezierCurve	(72.159, 67.286, 72.53, 67.46, 72.88, 67.59);
			path->addLine			(76.225, 68.713);
			path->addBezierCurve	(73.005, 77.835, 65.107, 84.766, 55.413, 86.703);
			path->addLine			(55.437, 27.429);
			path->addBezierCurve	(60.193, 25.409, 63.481, 20.699, 63.481, 15.24);
			path->addBezierCurve	(63.481, 7.918, 57.56, 2, 50.246, 2);
			path->addBezierCurve	(42.948, 2, 37.012, 7.915, 37.012, 15.24);
			path->addBezierCurve	(37.012, 20.699, 40.403, 25.409, 45.138, 27.429);
			path->addLine			(45.162, 86.788);
			path->addBezierCurve	(35.248, 85.012, 27.047, 78.003, 23.75, 68.716);
			path->addLine			(27.117, 67.594);
			path->addBezierCurve	(27.485, 67.463, 27.834, 67.289, 28.169, 67.046);
			path->addBezierCurve	(29.878, 65.769, 30.261, 63.351, 28.984, 61.636);
			path->addBezierCurve	(28.789, 61.381, 28.592, 61.162, 28.361, 60.964);
			path->addLine			(15.008, 48.451);
			path->addBezierCurve	(14.336, 47.721, 13.363, 47.256, 12.316, 47.256);
			path->addBezierCurve	(10.541, 47.256, 9.044, 48.545, 8.74, 50.236);
			path->addLine			(5.83, 68.287);
			path->addBezierCurve	(5.76, 68.567, 5.736, 68.88, 5.736, 69.19);
			path->addBezierCurve	(5.736, 71.328, 7.475, 73.067, 9.613, 73.067);
			path->addBezierCurve	(10.017, 73.067, 10.392, 73.019, 10.759, 72.894);
			path->addLine			(13.883, 71.872);
			path->addBezierCurve	(18.885, 87.028, 33.15, 98, 50, 98);
			path->addBezierCurve	(66.844, 98, 81.127, 87.031, 86.111, 71.848);
			path->addLine			(89.253, 72.894);
			path->addBezierCurve	(89.602, 73.019, 89.983, 73.067, 90.381, 73.067);
			path->addBezierCurve	(92.525, 73.067, 94.258, 71.328, 94.258, 69.19);
			path->addBezierCurve	(94.264, 68.877, 94.24, 68.561, 94.167, 68.284);
			break;
		}
		case kType2:
		{
			path->beginSubpath (44.199, 8.18);
			path->addBezierCurve (44.224, 1.102, 55.205, 1.102, 55.205, 8.388);
			path->addLine (55.205, 38.115);
			path->addLine (98, 63.14);
			path->addLine (98, 74.129);
			path->addLine (55.409, 60.495);
			path->addLine (55.409, 82.701);
			path->addLine (65.256, 90.221);
			path->addLine (65.256, 98.898);
			path->addLine (50.06, 94.309);
			path->addLine (34.863, 98.898);
			path->addLine (34.863, 90.221);
			path->addLine (44.616, 82.701);
			path->addLine (44.616, 60.495);
			path->addLine (2, 74.129);
			path->addLine (2, 63.14);
			path->addLine (44.199, 38.115);
			path->addLine (44.199, 8.18);
			path->closeSubpath ();
			break;
		}
		case kType3:
		{
			path->beginSubpath (50.048, 17.644);
			path->addBezierCurve (55.097, 17.644, 59.23, 14.158, 59.23, 9.822);
			path->addBezierCurve (59.23, 5.509, 55.097, 2, 50.048, 2);
			path->addBezierCurve (44.986, 2, 40.867, 5.509, 40.867, 9.822);
			path->addBezierCurve (40.853, 14.158, 44.973, 17.644, 50.048, 17.644);
			path->beginSubpath (37.567, 93.605);
			path->addBezierCurve (37.567, 96.027, 39.883, 98, 42.726, 98);
			path->addBezierCurve (45.569, 98, 47.885, 96.027, 47.885, 93.605);
			path->addLine (47.885, 56.764);
			path->addLine (52.212, 56.764);
			path->addLine (52.212, 93.605);
			path->addBezierCurve (52.212, 96.027, 54.5, 98, 57.343, 98);
			path->addBezierCurve (60.187, 98, 62.503, 96.027, 62.503, 93.605);
			path->addLine (62.544, 30.097);
			path->addLine (66.816, 30.097);
			path->addLine (66.816, 53.503);
			path->addBezierCurve (66.816, 58.229, 74.028, 58.229, 74, 53.503);
			path->addLine (74, 29.577);
			path->addBezierCurve (74, 24.39, 69.271, 19.309, 62.128, 19.309);
			path->addLine (37.733, 19.274);
			path->addBezierCurve (31.229, 19.274, 26, 23.823, 26, 29.435);
			path->addLine (26, 53.503);
			path->addBezierCurve (26, 58.17, 33.24, 58.17, 33.24, 53.503);
			path->addLine (33.24, 30.097);
			path->addLine (37.608, 30.097);
			path->addLine (37.567, 93.605);
			path->closeSubpath ();
			break;
		}
	}
	
	return path;
}

//------------------------------------------------------------------------
void GraphicsView::animationStart (CView* view, const char* name)
{
}

//------------------------------------------------------------------------
void GraphicsView::animationTick (CView* view, const char* name, float pos)
{
	if (strcmp (name, "Rotation") == 0 || strcmp (name, "RotateBack") == 0)
	{
		setRotation (360.f*pos);
	}
	else if (strcmp (name, "Saturation") == 0)
	{
		if (Bitmap::hueSaturateValue (getBackground (), 20., 0.01, 0.005))
			invalid ();
	}
}

//------------------------------------------------------------------------
void GraphicsView::animationFinished (CView* view, const char* name, bool wasCanceled)
{
	if (strcmp (name, "Rotation") == 0)
	{
		if (wasCanceled)
		{
			if (pathRotation > 0.f && pathRotation < 180.f)
			{
				remember ();
				InterpolationTimingFunction* tf = new InterpolationTimingFunction (100, pathRotation/360.f, 0);
				addAnimation ("RotateBack", this, tf);
			}
			else if (pathRotation >= 180.f)
			{
				remember ();
				InterpolationTimingFunction* tf = new InterpolationTimingFunction (100, pathRotation/360.f, 1);
				addAnimation ("RotateBack", this, tf);
			}
		}
	}
}

//------------------------------------------------------------------------
bool GraphicsView::attached (CView* parent)
{
	bool result = CView::attached (parent);
	if (result)
	{
//		remember ();
//		addAnimation ("Saturation", this, new LinearTimingFunction (15000));
	}
	return result;
}

//------------------------------------------------------------------------
bool GraphicsView::removed (CView* parent)
{
	for (int32_t i = 0; i < kNumPaths; i++)
	{
		if (path[i])
		{
			path[i]->forget ();
			path[i] = 0;
		}
	}
	if (gradient)
	{
		gradient->forget ();
		gradient = 0;
	}
	return CView::removed (parent);
}

//------------------------------------------------------------------------
void GraphicsView::setPathOne (int32_t index)
{
	if (index < kNumPaths)
	{
		path1Index = index;
		invalid ();
	}
}

//------------------------------------------------------------------------
void GraphicsView::setPathTwo (int32_t index)
{
	if (index < kNumPaths)
	{
		path2Index = index;
		invalid ();
	}
}

} // namespace
