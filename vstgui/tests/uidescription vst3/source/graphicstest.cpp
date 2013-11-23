//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 4.2
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2013, Steinberg Media Technologies, All Rights Reserved
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
	
	void setRotation (double angle) { pathRotation = angle; invalid (); }
	void setFillGradient (bool state) { fillGradient = state; invalid (); }
	void setStrokePath (bool state) { strokePath = state; invalid (); }
	void setPathOne (int32_t index);
	void setPathTwo (int32_t index);
	
	void draw (CDrawContext *pContext);

	bool attached (CView* parent);
	bool removed (CView* parent);

	// IAnimationTarget
	void animationStart (CView* view, const char* name);
	void animationTick (CView* view, const char* name, float pos);
	void animationFinished (CView* view, const char* name, bool wasCanceled);
	
	enum {
		kStarPath,
		kBezierPath,
		kArcPath,
		kNumPaths
	};

protected:
	static CGraphicsPath* buildStarPath (CDrawContext *pContext);
	static CGraphicsPath* buildBezierPath (CDrawContext *pContext);
	static CGraphicsPath* buildArcPath (CDrawContext *pContext);

	CBitmap* bitmap2;

	CGraphicsPath* path[kNumPaths];
	int32_t path1Index;
	int32_t path2Index;

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
class GraphicsViewController : public DelegationController, public CBaseObject
{
public:
	GraphicsViewController (IController* controller) : DelegationController (controller), graphicsView (0) {}
	~GraphicsViewController ();

	CView* verifyView (CView* view, const UIAttributes& attributes, IUIDescription* description);
	CControlListener* getControlListener (const char* controlTagName);
	void valueChanged (CControl* pControl);

	CMessageResult notify (CBaseObject* object, IdStringPtr message);
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
		slp->appendString (USTRING("Star"));
		slp->appendString (USTRING("Bezier"));
		slp->appendString (USTRING("Arc"));
		uiParameters.addParameter (slp);
		slp = new StringListParameter (USTRING("GraphicsPath2"), 20001);
		slp->appendString (USTRING("Star"));
		slp->appendString (USTRING("Bezier"));
		slp->appendString (USTRING("Arc"));
		uiParameters.addParameter (slp);
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
GraphicsViewController::~GraphicsViewController ()
{
	for (std::list<CControl*>::const_iterator it = controls.begin (); it != controls.end (); it++)
		(*it)->removeDependency (this);
}

//------------------------------------------------------------------------
CView* GraphicsViewController::verifyView (CView* view, const UIAttributes& attributes, IUIDescription* description)
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
CControlListener* GraphicsViewController::getControlListener (const char* controlTagName)
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
			graphicsView->setPathOne (pControl->getValue ());
			break;
		}
		case 20001:
		{
			graphicsView->setPathTwo (pControl->getValue ());
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
, pathRotation (0)
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
	CCoord x,y;
	getFrame ()->getPosition (x, y);
	CView::draw (pContext);
	if (path[kStarPath] == 0)
		path[kStarPath] = buildStarPath (pContext);
	if (path[kBezierPath] == 0)
		path[kBezierPath] = buildBezierPath (pContext);
	if (path[kArcPath] == 0)
		path[kArcPath] = buildArcPath (pContext);
	if (path[path1Index] == 0 || path[path2Index] == 0)
		return;
	CGraphicsPath* drawPath = pContext->createGraphicsPath ();
	if (drawPath)
	{
		drawPath->addPath (*path[path1Index]);
		drawPath->closeSubpath ();
		CGraphicsTransform t;
		t.rotate (90.-pathRotation*2.);
		t.scale (1.3, 1.3);
		drawPath->addPath (*path[path2Index], &t);

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
CGraphicsPath* GraphicsView::buildStarPath (CDrawContext *pContext)
{
	CGraphicsPath* starPath = pContext->createGraphicsPath ();
	if (starPath)
	{
		starPath->beginSubpath (CPoint (-0.4, -0.1));
		starPath->addLine (CPoint (-0.1, -0.1));
		starPath->addLine (CPoint (0.0, -0.45));
		starPath->addLine (CPoint (0.1, -0.1));
		starPath->addLine (CPoint (0.4, -0.1));
		starPath->addLine (CPoint (0.1, 0.1));
		starPath->addLine (CPoint (0.3, 0.45));
		starPath->addLine (CPoint (0.0, 0.25));
		starPath->addLine (CPoint (-0.3, 0.45));
		starPath->addLine (CPoint (-0.1, 0.1));
		starPath->addLine (CPoint (-0.4, -0.1));
		//starPath->closeSubpath ();
	}
	return starPath;
}

//------------------------------------------------------------------------
CGraphicsPath* GraphicsView::buildBezierPath (CDrawContext *pContext)
{
	CGraphicsPath* bezierPath = pContext->createGraphicsPath ();
	if (bezierPath)
	{
		bezierPath->beginSubpath (CPoint (-0.5, -0.5));
		bezierPath->addBezierCurve (CPoint (0, -0.2), CPoint (0, -0.2), CPoint (0.5, -0.5));
		bezierPath->addBezierCurve (CPoint (0.4, 0), CPoint (0.4, 0), CPoint (0.5, 0.5));
		bezierPath->addBezierCurve (CPoint (0, 0.2), CPoint (0, 0.2), CPoint (-0.5, 0.5));
		bezierPath->addBezierCurve (CPoint (-0.4, 0), CPoint (-0.4, 0), CPoint (-0.5, -0.5));
		bezierPath->closeSubpath ();
	}
	return bezierPath;
}

//------------------------------------------------------------------------
CGraphicsPath* GraphicsView::buildArcPath (CDrawContext *pContext)
{
	CGraphicsPath* arcPath = pContext->createGraphicsPath ();
	if (arcPath)
	{
		CRect r (0.,0.,0.5,0.5);
#if 1
		arcPath->addEllipse (r);
		arcPath->closeSubpath ();
#else
		arcPath->addArc (r, 0, 30, false);
//		arcPath->closeSubpath ();
		arcPath->addArc (r, 60, 90, false);
//		arcPath->closeSubpath ();
		arcPath->addArc (r, 120, 150, false);
//		arcPath->closeSubpath ();
		arcPath->addArc (r, 180, 210, false);
//		arcPath->closeSubpath ();
		arcPath->addArc (r, 240, 270, false);
//		arcPath->closeSubpath ();
		arcPath->addArc (r, 200, 330, false);
//		arcPath->closeSubpath ();
#endif
	}
	return arcPath;
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
