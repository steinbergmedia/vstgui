//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework not only for VST plugins :
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

#include "cshadowviewcontainer.h"
#include "coffscreencontext.h"
#include "cbitmapfilter.h"
#include "cframe.h"
#include "cbitmap.h"
#include <cassert>

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
CShadowViewContainer::~CShadowViewContainer ()
{
	unregisterViewContainerListener (this);
}

//-----------------------------------------------------------------------------
bool CShadowViewContainer::removed (CView* parent)
{
	getFrame ()->unregisterScaleFactorChangedListeneer (this);
	setBackground (0);
	return CViewContainer::removed (parent);
}

//-----------------------------------------------------------------------------
bool CShadowViewContainer::attached (CView* parent)
{
	if (CViewContainer::attached (parent))
	{
		invalidateShadow ();
		getFrame ()->registerScaleFactorChangedListeneer (this);
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
void CShadowViewContainer::onScaleFactorChanged (CFrame* frame)
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
static std::vector<int32_t> boxesForGauss (double sigma, uint16_t numBoxes)
{
	std::vector<int32_t> boxes;
	double ideal = std::sqrt ((12 * sigma * sigma / numBoxes) + 1);
	uint16_t l = static_cast<uint16_t> (std::floor (ideal));
	if (l % 2 == 0)
		l--;
	int32_t u = l + 2;
	ideal = ((12. * sigma * sigma) - (numBoxes * l * l) - (4. * numBoxes * l) - (3. * numBoxes)) / ((-4. * l) - 4.);
	int32_t m = static_cast<int32_t> (std::floor (ideal));
	for (int32_t i = 0; i < numBoxes; ++i)
		boxes.push_back (i < m ? l : u);
	return boxes;
}

//-----------------------------------------------------------------------------
static bool isUniformScaled (const CGraphicsTransform& matrix)
{
	return matrix.m11 == matrix.m22;
}

//-----------------------------------------------------------------------------
void CShadowViewContainer::drawRect (CDrawContext* pContext, const CRect& updateRect)
{
	double scaleFactor = pContext->getScaleFactor ();
	CGraphicsTransform matrix = pContext->getCurrentTransform ();
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
		
		SharedPointer<COffscreenContext> offscreenContext = owned (COffscreenContext::create (getFrame (), width, height, scaleFactor));
		if (offscreenContext)
		{
			offscreenContext->beginDraw ();
			CDrawContext::Transform transform (*offscreenContext, CGraphicsTransform ().translate (-getViewSize ().left - shadowOffset.x, -getViewSize ().top - shadowOffset.y));
			dontDrawBackground = true;
			CViewContainer::draw (offscreenContext);
			dontDrawBackground = false;
			offscreenContext->endDraw ();
			CBitmap* bitmap = offscreenContext->getBitmap ();
			if (bitmap)
			{
				setBackground (bitmap);
				SharedPointer<BitmapFilter::IFilter> setColorFilter = owned (BitmapFilter::Factory::getInstance ().createFilter (BitmapFilter::Standard::kSetColor));
				if (setColorFilter)
				{
					setColorFilter->setProperty (BitmapFilter::Standard::Property::kInputBitmap, bitmap);
					setColorFilter->setProperty (BitmapFilter::Standard::Property::kInputColor, kBlackCColor);
					setColorFilter->setProperty (BitmapFilter::Standard::Property::kIgnoreAlphaColorValue, (int32_t)1);
					if (setColorFilter->run (true))
					{
						SharedPointer<BitmapFilter::IFilter> boxBlurFilter = owned (BitmapFilter::Factory::getInstance ().createFilter (BitmapFilter::Standard::kBoxBlur));
						if (boxBlurFilter)
						{
							std::vector<int32_t> boxSizes = boxesForGauss (shadowBlurSize, 3);
							boxBlurFilter->setProperty (BitmapFilter::Standard::Property::kInputBitmap, bitmap);
							boxBlurFilter->setProperty (BitmapFilter::Standard::Property::kRadius, boxSizes[0]);
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

				CViewContainer::drawRect (pContext, updateRect);
			}
		}
	}
	else
	{
		CViewContainer::drawRect (pContext, updateRect);
	}
}

//-----------------------------------------------------------------------------
void CShadowViewContainer::drawBackgroundRect (CDrawContext* pContext, const CRect& _updateRect)
{
	if (!dontDrawBackground)
	{
		float tmp = pContext->getGlobalAlpha ();
		pContext->setGlobalAlpha (tmp * shadowIntensity);
		CViewContainer::drawBackgroundRect (pContext, _updateRect);
		pContext->setGlobalAlpha (tmp);
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
void CShadowViewContainer::viewContainerViewAdded (CViewContainer* container, CView* view)
{
	vstgui_assert (container == this);
	invalidateShadow ();
}

//-----------------------------------------------------------------------------
void CShadowViewContainer::viewContainerViewRemoved (CViewContainer* container, CView* view)
{
	vstgui_assert (container == this);
	invalidateShadow ();
}

//-----------------------------------------------------------------------------
void CShadowViewContainer::viewContainerViewZOrderChanged (CViewContainer* container, CView* view)
{
	vstgui_assert (container == this);
	invalidateShadow ();
}

} // namespace
