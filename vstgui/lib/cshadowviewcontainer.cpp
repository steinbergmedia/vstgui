// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "cshadowviewcontainer.h"
#include "coffscreencontext.h"
#include "cbitmapfilter.h"
#include "cframe.h"
#include "cbitmap.h"
#include <cassert>
#include <array>

namespace VSTGUI {

//-----------------------------------------------------------------------------
CShadowViewContainer::CShadowViewContainer (const CRect& size)
: CViewContainer (size)
, dontDrawBackground (false)
, shadowIntensity (0.3f)
, shadowBlurSize (4)
, scaleFactorUsed (0.)
{
	registerViewContainerListener (this);
}

//-----------------------------------------------------------------------------
CShadowViewContainer::CShadowViewContainer (const CShadowViewContainer& copy)
: CViewContainer (copy)
, dontDrawBackground (false)
, shadowIntensity (copy.shadowIntensity)
, shadowBlurSize (copy.shadowBlurSize)
, scaleFactorUsed (0.)
{
	registerViewContainerListener (this);
}

//------------------------------------------------------------------------
CShadowViewContainer::~CShadowViewContainer () noexcept = default;

//------------------------------------------------------------------------
void CShadowViewContainer::beforeDelete ()
{
	unregisterViewContainerListener (this);
	CViewContainer::beforeDelete ();
}

//-----------------------------------------------------------------------------
bool CShadowViewContainer::removed (const SharedPointer<CViewContainer>& parent)
{
	if (auto frame = getFrame ())
		frame->unregisterScaleFactorChangedListener (this);
	setBackground (nullptr);
	return CViewContainer::removed (parent);
}

//-----------------------------------------------------------------------------
bool CShadowViewContainer::attached (const SharedPointer<CViewContainer>& parent)
{
	if (CViewContainer::attached (parent))
	{
		invalidateShadow ();
		if (auto frame = getFrame ())
			frame->registerScaleFactorChangedListener (this);
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
void CShadowViewContainer::onScaleFactorChanged (CFrame& frame, double newScaleFactor)
{
	invalidateShadow ();
}

//-----------------------------------------------------------------------------
void CShadowViewContainer::setShadowOffset (const CPoint& offset)
{
	if (shadowOffset != offset)
	{
		shadowOffset = offset;
		invalidateShadow ();
	}
}

//-----------------------------------------------------------------------------
void CShadowViewContainer::setShadowIntensity (float intensity)
{
	if (shadowIntensity != intensity)
	{
		shadowIntensity = intensity;
		invalid ();
	}
}

//-----------------------------------------------------------------------------
void CShadowViewContainer::setShadowBlurSize (double size)
{
	if (shadowBlurSize != size)
	{
		shadowBlurSize = size;
		invalidateShadow ();
	}
}

//-----------------------------------------------------------------------------
void CShadowViewContainer::invalidateShadow ()
{
	scaleFactorUsed = 0.;
	invalid ();
}

//-----------------------------------------------------------------------------
CMessageResult CShadowViewContainer::notify (CBaseObject* sender, IdStringPtr message)
{
	if (message == kMsgViewSizeChanged)
		invalidateShadow ();
	return CViewContainer::notify(sender, message);
}

//-----------------------------------------------------------------------------
template<int32_t numBoxes>
static std::array<int32_t, numBoxes> boxesForGauss (double sigma)
{
	std::array<int32_t, numBoxes> boxes;
	double ideal = std::sqrt ((12 * sigma * sigma / numBoxes) + 1);
	uint16_t l = static_cast<uint16_t> (std::floor (ideal));
	if (l % 2 == 0)
		l--;
	int32_t u = l + 2;
	ideal = ((12. * sigma * sigma) - (numBoxes * l * l) - (4. * numBoxes * l) - (3. * numBoxes)) / ((-4. * l) - 4.);
	int32_t m = static_cast<int32_t> (std::floor (ideal));
	for (int32_t i = 0; i < static_cast<int32_t> (numBoxes); ++i)
		boxes[i] = (i < m ? l : u);
	return boxes;
}

//-----------------------------------------------------------------------------
static bool isUniformScaled (const CGraphicsTransform& matrix)
{
	return matrix.m11 == matrix.m22;
}

//-----------------------------------------------------------------------------
void CShadowViewContainer::drawRect (CDrawContext& context, const CRect& updateRect)
{
	double scaleFactor = context.getScaleFactor ();
	CGraphicsTransform matrix = context.getCurrentTransform ();
	if (isUniformScaled (matrix))
	{
		double matrixScale = std::floor (matrix.m11 + 0.5);
		if (matrixScale != 0.)
			scaleFactor *= matrixScale;
	}
	if (scaleFactor != scaleFactorUsed && getWidth () > 0. && getHeight () > 0.)
	{
		scaleFactorUsed = scaleFactor;
		CCoord width = getWidth ();
		CCoord height = getHeight ();
		
		if (auto offscreenContext = COffscreenContext::create ({width, height}, scaleFactor))
		{
			offscreenContext->beginDraw ();
			CDrawContext::Transform transform (
				*offscreenContext.get (),
				CGraphicsTransform ().translate (-getViewSize ().left - shadowOffset.x,
												 -getViewSize ().top - shadowOffset.y));
			dontDrawBackground = true;
			CViewContainer::draw (*offscreenContext.get ());
			dontDrawBackground = false;
			offscreenContext->endDraw ();
			auto bitmap = offscreenContext->getBitmap ();
			if (bitmap)
			{
				setBackground (bitmap);
				SharedPointer<BitmapFilter::IFilter> setColorFilter = owned (BitmapFilter::Factory::getInstance ().createFilter (BitmapFilter::Standard::kSetColor));
				if (setColorFilter)
				{
					setColorFilter->setProperty (BitmapFilter::Standard::Property::kInputBitmap,
												 bitmap.cast<IReference> ());
					setColorFilter->setProperty (BitmapFilter::Standard::Property::kInputColor, kBlackCColor);
					setColorFilter->setProperty (BitmapFilter::Standard::Property::kIgnoreAlphaColorValue, (int32_t)1);
					if (setColorFilter->run (true))
					{
						SharedPointer<BitmapFilter::IFilter> boxBlurFilter = owned (BitmapFilter::Factory::getInstance ().createFilter (BitmapFilter::Standard::kBoxBlur));
						if (boxBlurFilter)
						{
							auto boxSizes = boxesForGauss<3> (shadowBlurSize);
							boxBlurFilter->setProperty (
								BitmapFilter::Standard::Property::kInputBitmap,
								bitmap.cast<IReference> ());
							boxBlurFilter->setProperty (BitmapFilter::Standard::Property::kRadius, boxSizes[0]);
							boxBlurFilter->setProperty (BitmapFilter::Standard::Property::kAlphaChannelOnly, 1);
							if (boxBlurFilter->run (true))
							{
								boxBlurFilter->setProperty (BitmapFilter::Standard::Property::kRadius, boxSizes[1]);
								boxBlurFilter->run (true);
								boxBlurFilter->setProperty (BitmapFilter::Standard::Property::kRadius, boxSizes[2]);
								boxBlurFilter->run (true);
							}
						}
					}
				}

				CViewContainer::drawRect (context, updateRect);
			}
		}
	}
	else
	{
		CViewContainer::drawRect (context, updateRect);
	}
}

//-----------------------------------------------------------------------------
void CShadowViewContainer::drawBackgroundRect (CDrawContext& context, const CRect& _updateRect)
{
	if (!dontDrawBackground)
	{
		float tmp = context.getGlobalAlpha ();
		context.setGlobalAlpha (tmp * shadowIntensity);
		CViewContainer::drawBackgroundRect (context, _updateRect);
		context.setGlobalAlpha (tmp);
	}
}

//-----------------------------------------------------------------------------
void CShadowViewContainer::setViewSize (const CRect& rect, bool invalid)
{
	if (getViewSize () != rect)
	{
		bool diffSize = (getWidth () != rect.getWidth () || getHeight () != rect.getHeight ());
		CViewContainer::setViewSize (rect, invalid);
		if (diffSize)
			invalidateShadow ();
	}
}

//-----------------------------------------------------------------------------
void CShadowViewContainer::viewContainerViewAdded (CViewContainer& container, CView& view)
{
	vstgui_assert (&container == this);
	invalidateShadow ();
}

//-----------------------------------------------------------------------------
void CShadowViewContainer::viewContainerViewRemoved (CViewContainer& container, CView& view)
{
	vstgui_assert (&container == this);
	invalidateShadow ();
}

//-----------------------------------------------------------------------------
void CShadowViewContainer::viewContainerViewZOrderChanged (CViewContainer& container, CView& view)
{
	vstgui_assert (&container == this);
	invalidateShadow ();
}

} // VSTGUI
