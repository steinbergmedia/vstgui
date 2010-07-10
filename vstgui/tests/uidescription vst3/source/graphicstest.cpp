/*
 *  graphicstest.cpp
 *  uidescription test
 *
 *  Created by Arne Scheffler on 3/16/10.
 *  Copyright 2010 Arne Scheffler. All rights reserved.
 *
 */

#include "graphicstest.h"
#include "vstgui/lib/animation/animations.h"
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
	
	void setRotation (double angle) { pathRotation = angle; invalid (); }
	void setFillGradient (bool state) { fillGradient = state; invalid (); }
	void setStrokePath (bool state) { strokePath = state; invalid (); }
	
	void draw (CDrawContext *pContext);

	bool attached (CView* parent);
	bool removed (CView* parent);

	// IAnimationTarget
	void animationStart (CView* view, const char* name);
	void animationTick (CView* view, const char* name, float pos);
	void animationFinished (CView* view, const char* name, bool wasCanceled);
protected:
	void buildStarPath (CDrawContext *pContext);
	void buildBezierPath (CDrawContext *pContext);

	CBitmap* bitmap2;
	CGraphicsPath* starPath;
	CGraphicsPath* bezierPath;
	CGradient* gradient;
	double pathRotation;
	bool fillGradient;
	bool strokePath;
};

//------------------------------------------------------------------------
class FadeViewContainer : public CViewContainer
{
public:
	FadeViewContainer () : CViewContainer (CRect (0, 0, 0, 0)) { setAlphaValue (0.3f); }
	
	CMouseEventResult onMouseEntered (CPoint &where, const CButtonState& buttons)
	{
		addAnimation ("AlphaAnimation", new Animation::AlphaValueAnimation (1.f), new Animation::LinearTimingFunction (100));
		return kMouseEventHandled;
	}

	CMouseEventResult onMouseExited (CPoint &where, const CButtonState& buttons)
	{
		addAnimation ("AlphaAnimation", new Animation::AlphaValueAnimation (0.3f), new Animation::LinearTimingFunction (100));
		return kMouseEventHandled;
	}
};

//------------------------------------------------------------------------
class GraphicsViewController : public DelegationController
{
public:
	GraphicsViewController (IController* controller) : DelegationController (controller), graphicsView (0) {}

	CView* verifyView (CView* view, const UIAttributes& attributes, IUIDescription* description);
	CControlListener* getControlListener (const char* controlTagName);
	void valueChanged (CControl* pControl);

protected:
	GraphicsView* graphicsView;
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
//		StringListParameter* slp = new StringListParameter (USTRING("Animation Switch"), 20000);
//		slp->appendString (USTRING("On"));
//		slp->appendString (USTRING("Off"));
//		uiParameters.addParameter (slp);
	}
	return res;
}

//------------------------------------------------------------------------
IPlugView* PLUGIN_API GraphicsTestController::createView (FIDString name)
{
	if (strcmp (name, ViewType::kEditor) == 0)
	{
		return new VST3Editor (this, "view", "GraphicsTest.uidesc");
	}
	return 0;
}

//------------------------------------------------------------------------
CView* GraphicsTestController::createCustomView (const char* name, const UIAttributes& attributes, IUIDescription* description, VST3Editor* editor)
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
IController* GraphicsTestController::createSubController (const char* name, IUIDescription* description, VST3Editor* editor)
{
	if (strcmp (name, "GraphicsViewController") == 0)
		return new GraphicsViewController (editor);
	return 0;
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
CView* GraphicsViewController::verifyView (CView* view, const UIAttributes& attributes, IUIDescription* description)
{
	if (graphicsView == 0)
		graphicsView = dynamic_cast<GraphicsView*> (view);
	return DelegationController::verifyView (view, attributes, description);
}

//------------------------------------------------------------------------
CControlListener* GraphicsViewController::getControlListener (const char* controlTagName)
{
	if (strcmp (controlTagName, "Rotate") == 0 
	 || strcmp (controlTagName, "FillGradient") == 0
	 || strcmp (controlTagName, "StrokePath") == 0)
	{
		return this;
	}
	return DelegationController::getControlListener (controlTagName);
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
	}
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
GraphicsView::GraphicsView ()
: CView (CRect (0, 0, 0, 0))
, starPath (0)
, bezierPath (0)
, gradient (0)
, pathRotation (0)
, fillGradient (false)
, strokePath (false)
, bitmap2 (0)
{
}

//------------------------------------------------------------------------
void GraphicsView::draw (CDrawContext *pContext)
{
	CView::draw (pContext);
	buildStarPath (pContext);
	buildBezierPath (pContext);
	if (starPath == 0 || bezierPath == 0)
		return;
	CGraphicsPath* drawPath = pContext->createGraphicsPath ();
	if (drawPath)
	{
		drawPath->addPath (*bezierPath);
		CGraphicsTransform t;
		t.rotate (90.-pathRotation*2.);
		t.scale (1.3, 1.3);
		drawPath->addPath (*starPath, &t);

		pContext->setDrawMode (kAntialias);
		pContext->setLineWidth (getWidth () / 100.);
		pContext->setFrameColor (kRedCColor);
		pContext->setFillColor (kBlueCColor);
		double lengths[] = {1, 3, 4, 2};
		CLineStyle lineStyle (CLineStyle::kLineCapSquare, CLineStyle::kLineJoinBevel, 0, 4, lengths);
		pContext->setLineStyle (lineStyle);
		CGraphicsTransform t2;
		t2.translate (size.left + getWidth ()/2., size.top + getHeight ()/2.);
		t2.scale (getWidth ()/1.5, getHeight ()/1.5);
		t2.rotate (pathRotation);
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
void GraphicsView::buildStarPath (CDrawContext *pContext)
{
	if (starPath)
		return;
	starPath = pContext->createGraphicsPath ();
	if (starPath)
	{
		starPath->addLine (CPoint (-0.4, -0.1), CPoint (-0.1, -0.1));
		starPath->addLine (starPath->getCurrentPosition (), CPoint (0.0, -0.45));
		starPath->addLine (starPath->getCurrentPosition (), CPoint (0.1, -0.1));
		starPath->addLine (starPath->getCurrentPosition (), CPoint (0.4, -0.1));
		starPath->addLine (starPath->getCurrentPosition (), CPoint (0.1, 0.1));
		starPath->addLine (starPath->getCurrentPosition (), CPoint (0.3, 0.45));
		starPath->addLine (starPath->getCurrentPosition (), CPoint (0.0, 0.25));
		starPath->addLine (starPath->getCurrentPosition (), CPoint (-0.3, 0.45));
		starPath->addLine (starPath->getCurrentPosition (), CPoint (-0.1, 0.1));
		starPath->addLine (starPath->getCurrentPosition (), CPoint (-0.4, -0.1));
		//starPath->closeSubpath ();
	}
}

//------------------------------------------------------------------------
void GraphicsView::buildBezierPath (CDrawContext *pContext)
{
	if (bezierPath)
		return;
	bezierPath = pContext->createGraphicsPath ();
	if (bezierPath)
	{
		bezierPath->addCurve (CPoint (-0.5, -0.5), CPoint (0, -0.2), CPoint (0, -0.2), CPoint (0.5, -0.5));
		bezierPath->addCurve (bezierPath->getCurrentPosition (), CPoint (0.4, 0), CPoint (0.4, 0), CPoint (0.5, 0.5));
		bezierPath->addCurve (bezierPath->getCurrentPosition (), CPoint (0, 0.2), CPoint (0, 0.2), CPoint (-0.5, 0.5));
		bezierPath->addCurve (bezierPath->getCurrentPosition (), CPoint (-0.4, 0), CPoint (-0.4, 0), CPoint (-0.5, -0.5));
		bezierPath->closeSubpath ();
	}
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
		setRotation (360.*pos);
	}
	else if (strcmp (name, "Saturation") == 0)
	{
		if (Bitmap::hueSaturateValue (pBackground, 20., 0.01, 0.005))
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
			if (pathRotation > 0. && pathRotation < 180.)
			{
				remember ();
				InterpolationTimingFunction* tf = new InterpolationTimingFunction (100, pathRotation/360., 0);
				addAnimation ("RotateBack", this, tf);
			}
			else if (pathRotation >= 180.)
			{
				remember ();
				InterpolationTimingFunction* tf = new InterpolationTimingFunction (100, pathRotation/360., 1);
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
		remember ();
		addAnimation ("Saturation", this, new LinearTimingFunction (15000));
	}
	return result;
}

//------------------------------------------------------------------------
bool GraphicsView::removed (CView* parent)
{
	if (starPath)
	{
		starPath->forget ();
		starPath = 0;
	}
	if (bezierPath)
	{
		bezierPath->forget ();
		bezierPath = 0;
	}
	if (gradient)
	{
		gradient->forget ();
		gradient = 0;
	}
	return CView::removed (parent);
}

} // namespace
