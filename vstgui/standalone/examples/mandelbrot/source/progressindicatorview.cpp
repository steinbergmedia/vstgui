// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "vstgui/lib/animation/animations.h"
#include "vstgui/lib/animation/ianimationtarget.h"
#include "vstgui/lib/animation/timingfunctions.h"
#include "vstgui/lib/cdrawcontext.h"
#include "vstgui/lib/controls/ccontrol.h"
#include "vstgui/uidescription/detail/uiviewcreatorattributes.h"
#include "vstgui/uidescription/iviewcreator.h"
#include "vstgui/uidescription/uiattributes.h"
#include "vstgui/uidescription/uiviewfactory.h"
#include <chrono>

//------------------------------------------------------------------------
namespace Mandelbrot {

using namespace VSTGUI;

//------------------------------------------------------------------------
class ProgressIndicatorAnimation : public Animation::IAnimationTarget
{
public:
	void animationStart (CView* view, IdStringPtr name) override;
	void animationTick (CView* view, IdStringPtr name, float pos) override;
	void animationFinished (CView* view, IdStringPtr name, bool wasCanceled) override;
};

//------------------------------------------------------------------------
class ProgressIndicatorView : public CControl
{
public:
	ProgressIndicatorView () : CControl ({}, nullptr, -1) {}

	void draw (CDrawContext* context) override;
	void setRotation (double r);
	double getRotation () const { return rotation; }
	void setRotationTime (uint32_t t);
	uint32_t getRotationTime () const { return rotationTime; }
	CLASS_METHODS_NOCOPY (ProgressIndicatorView, CControl)
private:
	void setValue (float val) override;

	double rotation {0.};
	uint32_t rotationTime {1000};
};

//------------------------------------------------------------------------
void ProgressIndicatorView::draw (CDrawContext* context)
{
	CDrawContext::Transform t (
	    *context, CGraphicsTransform ().rotate (rotation, getViewSize ().getCenter ()));
	CView::draw (context);
}

//------------------------------------------------------------------------
void ProgressIndicatorView::setRotation (double r)
{
	rotation = r;
	invalid ();
}

//------------------------------------------------------------------------
void ProgressIndicatorView::setRotationTime (uint32_t t)
{
	rotationTime = t;
}

//------------------------------------------------------------------------
void ProgressIndicatorView::setValue (float val)
{
	if (val == getValue ())
		return;
	CControl::setValue (val);
	if (!isAttached ())
		return;

	bool animActive = getValueNormalized () >= 0.5;
	if (animActive)
	{
		auto tf = new Animation::LinearTimingFunction (rotationTime);
		addAnimation ("Animation", new ProgressIndicatorAnimation (),
		              new Animation::RepeatTimingFunction (tf, -1, false));
		setAlphaValue (0.f);
		addAnimation ("Alpha", new Animation::AlphaValueAnimation (1.f),
		              new Animation::LinearTimingFunction (200));
	}
	else
	{
		addAnimation ("Alpha", new Animation::AlphaValueAnimation (0.f),
		              new Animation::LinearTimingFunction (80),
		              [] (CView* view, const IdStringPtr, Animation::IAnimationTarget*) {
			              view->removeAnimation ("Animation");
			          });
	}
}

//------------------------------------------------------------------------
void ProgressIndicatorAnimation::animationStart (CView* view, IdStringPtr name)
{
	static_cast<ProgressIndicatorView*> (view)->setRotation (0.);
}

//------------------------------------------------------------------------
void ProgressIndicatorAnimation::animationTick (CView* view, IdStringPtr name, float pos)
{
	static_cast<ProgressIndicatorView*> (view)->setRotation (360. * static_cast<double> (pos));
}

//------------------------------------------------------------------------
void ProgressIndicatorAnimation::animationFinished (CView* view, IdStringPtr name, bool wasCanceled)
{
}

//-----------------------------------------------------------------------------
class ProgressIndicatorViewCreator : public ViewCreatorAdapter
{
public:
	const std::string kAttrRotation {"rotation"};
	const std::string kAttrRotationTime {"rotation-time"};

	ProgressIndicatorViewCreator () { UIViewFactory::registerViewCreator (*this); }
	IdStringPtr getViewName () const override { return "ProgressIndicatorView"; }
	IdStringPtr getBaseViewName () const override { return UIViewCreator::kCControl; }
	UTF8StringPtr getDisplayName () const override { return "Progress Indicator View"; }
	CView* create (const UIAttributes& attributes, const IUIDescription* description) const override
	{
		return new ProgressIndicatorView ();
	}
	bool apply (CView* view, const UIAttributes& attributes,
	            const IUIDescription* description) const override
	{
		auto piv = dynamic_cast<ProgressIndicatorView*> (view);
		if (!piv)
			return false;
		double rotation = 0.;
		if (attributes.getDoubleAttribute (kAttrRotation, rotation))
			piv->setRotation (rotation);
		int32_t time = 0;
		if (attributes.getIntegerAttribute (kAttrRotationTime, time))
			piv->setRotationTime (static_cast<uint32_t> (time));
		return true;
	}
	bool getAttributeNames (std::list<std::string>& attributeNames) const override
	{
		attributeNames.emplace_back (kAttrRotation);
		attributeNames.emplace_back (kAttrRotationTime);
		return true;
	}
	AttrType getAttributeType (const std::string& attributeName) const override
	{
		if (attributeName == kAttrRotation)
			return kFloatType;
		if (attributeName == kAttrRotationTime)
			return kIntegerType;
		return kUnknownType;
	}
	bool getAttributeValue (CView* view, const std::string& attributeName, std::string& stringValue,
	                        const IUIDescription* desc) const override
	{
		auto piv = dynamic_cast<ProgressIndicatorView*> (view);
		if (!piv)
			return false;
		if (attributeName == kAttrRotation)
		{
			stringValue = std::to_string (piv->getRotation ());
			return true;
		}
		if (attributeName == kAttrRotationTime)
		{
			stringValue = std::to_string (piv->getRotationTime ());
			return true;
		}
		return false;
	}
	bool getAttributeValueRange (const std::string& attributeName, double& minValue,
	                             double& maxValue) const override
	{
		if (attributeName == kAttrRotation)
		{
			minValue = 0.;
			maxValue = 360.;
			return true;
		}
		return false;
	}
};
ProgressIndicatorViewCreator __gProgressIndicatorViewCreator;

//------------------------------------------------------------------------
} // Mandelbrot
