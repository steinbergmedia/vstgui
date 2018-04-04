// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "imageframesview.h"
#include "vstgui/lib/cdrawcontext.h"
#include "vstgui/lib/cdropsource.h"
#include "vstgui/lib/cframe.h"
#include "vstgui/lib/cscrollview.h"
#include "vstgui/lib/dragging.h"
#include <cassert>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace ImageStitcher {

//------------------------------------------------------------------------
ImageFramesView::ImageFramesView () : CView (CRect (0, 0, 10, 10))
{
	font = makeOwned<CFontDesc> (*kSystemFont);
	font->setSize (8);
}

//------------------------------------------------------------------------
void ImageFramesView::setDocContext (const DocumentContextPtr& dc)
{
	docContext = dc;
}

//------------------------------------------------------------------------
void ImageFramesView::setImageList (ImageList* list)
{
	imageList = list;
	updateViewSize ();
	invalid ();
}

//------------------------------------------------------------------------
void ImageFramesView::updateViewSize ()
{
	if (imageList)
	{
		CRect r = getViewSize ();
		if (imageList->empty ())
		{
			r.setSize ({0., 0.});
		}
		else
		{
			if (auto image = imageList->front ().bitmap)
			{
				auto size = image->getSize ();
				size.y += titleHeight;
				rowHeight = size.y;
				size.y *= imageList->size ();
				size.y += 15; // back list drop zone
				r.setSize (size);
			}
		}
		if (isAttached ())
		{
			if (auto scrollView = dynamic_cast<CScrollView*> (getParentView ()->getParentView ()))
			{
				auto parentSize = scrollView->getViewSize ();
				if (parentSize.getWidth () > r.getWidth ())
					r.setWidth (parentSize.getWidth ());
				if (parentSize.getHeight () > r.getHeight ())
					r.setHeight (parentSize.getHeight ());
			}
		}
		setMouseableArea (r);
		setViewSize (r);
	}
}

//------------------------------------------------------------------------
void ImageFramesView::parentSizeChanged ()
{
	updateViewSize ();
}

//------------------------------------------------------------------------
void ImageFramesView::drawRect (CDrawContext* context, const CRect& _updateRect)
{
	if (!imageList || imageList->empty ())
		return;
	auto topLeft = getViewSize ().getTopLeft ();
	CDrawContext::Transform tm (*context, CGraphicsTransform ().translate (topLeft));

	CRect updateRect (_updateRect);
	updateRect.offsetInverse (topLeft);

	context->setFillColor (selectionColor);
	context->setFontColor (kBlackCColor);
	context->setFont (font);
	context->setDrawMode (kAntiAliasing);

	CRect r;
	auto imageSize = imageList->front ().bitmap->getSize ();
	r.setSize (imageSize);
	r.setWidth (getWidth ());
	int32_t index = 0;
	for (auto& image : *imageList)
	{
		if (image.selected)
		{
			CRect sr (r);
			sr.bottom += titleHeight;
			if (updateRect.rectOverlap (sr))
				context->drawRect (sr, kDrawFilled);
		}
		CRect ir (r);
		ir.setWidth (imageSize.x);
		ir.offset (r.getWidth () / 2. - imageSize.x / 2., 0);
		if (updateRect.rectOverlap (ir))
			image.bitmap->draw (context, ir);
		if (!image.path.empty ())
		{
			CRect tr (r);
			tr.top = tr.bottom;
			tr.bottom += titleHeight - 1;
			if (updateRect.rectOverlap (tr))
			{
				auto name = getDisplayFilename (image.path);
				context->drawString (name.data (), tr);
			}
		}
		if (index == dropIndicatorPos)
		{
			context->setFrameColor (kRedCColor);
			context->setLineWidth (2.);
			context->drawLine (r.getTopLeft (), r.getTopRight ());
		}
		r.offset (0, rowHeight);
		++index;
	}
	if (dropIndicatorPos == static_cast<int32_t> (imageList->size ()))
	{
		context->setFrameColor (kRedCColor);
		context->setLineWidth (2.);
		context->drawLine (r.getTopLeft (), r.getTopRight ());
	}
}

//------------------------------------------------------------------------
int32_t ImageFramesView::posToIndex (CPoint where) const
{
	if (imageList->empty ())
		return 0;
	CRect r;
	auto size = imageList->front ().bitmap->getSize ();
	size.y += titleHeight;
	size.x = getWidth ();
	r.setSize (size);
	r.offset (getViewSize ().getTopLeft ());
	for (auto index = 0u; index < imageList->size (); ++index)
	{
		if (r.pointInside (where))
			return index;
		r.offset (0, r.getHeight ());
	}
	return static_cast<int32_t> (imageList->size ());
}

//------------------------------------------------------------------------
void ImageFramesView::selectExclusive (size_t index)
{
	for (auto i = 0u; i < imageList->size (); ++i)
	{
		auto& image = imageList->at (i);
		image.selected = (i == index);
	}
	invalid ();
}

//------------------------------------------------------------------------
void ImageFramesView::enlargeSelection (size_t index)
{
	// find nearest selected item
	int32_t nearest = std::numeric_limits<int32_t>::max ();
	for (auto i = 0u; i < imageList->size (); ++i)
	{
		auto& image = imageList->at (i);
		if (!image.selected)
			continue;
		auto diff = static_cast<int32_t> (i) - static_cast<int32_t> (index);
		if (std::abs (diff) < std::abs (nearest))
			nearest = diff;
	}
	if (nearest == std::numeric_limits<int32_t>::max ())
		nearest = -static_cast<int32_t> (index);

	if (nearest < 0)
	{
		for (auto i = index + nearest; i <= index; ++i)
			imageList->at (i).selected = true;
	}
	else if (nearest > 0)
	{
		for (auto i = index; i < index + nearest; ++i)
			imageList->at (i).selected = true;
	}
	invalid ();
}

//------------------------------------------------------------------------
CMouseEventResult ImageFramesView::onMouseDown (CPoint& where, const CButtonState& buttons)
{
	assert (imageList);
	if (buttons.isLeftButton ())
	{
		mouseDownPos = where;
		bool exclusive = buttons.getModifierState () != kControl;
		bool shift = buttons.getModifierState () == kShift;
		auto index = posToIndex (where);
		if (index >= 0 && index < static_cast<int32_t> (imageList->size ()))
		{
			if (shift)
			{
				enlargeSelection (index);
			}
			else if (exclusive)
			{
				if (!imageList->at (index).selected)
				{
					selectExclusive (index);
				}
			}
			else
			{
				imageList->at (index).selected = !imageList->at (index).selected;
				invalid ();
			}
		}
		return kMouseEventHandled;
	}
	return kMouseEventNotHandled;
}

static constexpr size_t DragPackageID = 'isdp';

//------------------------------------------------------------------------
static bool getIndicesFromDataPackage (IDataPackage* package, std::vector<size_t>* result = nullptr)
{
	if (package->getDataType (0) == IDataPackage::kBinary)
	{
		const void* buffer;
		IDataPackage::Type type;
		if (auto dataSize = package->getData (0, buffer, type))
		{
			auto data = reinterpret_cast<const size_t*> (buffer);
			if (data[0] == DragPackageID)
			{
				if (result)
				{
					for (auto index = 1u; index < dataSize / sizeof (size_t); ++index)
					{
						result->emplace_back (data[index]);
					}
				}
				return true;
			}
		}
	}
	return false;
}

//------------------------------------------------------------------------
std::vector<Path> getDragPngImagePaths (IDataPackage* drag)
{
	std::vector<Path> result;
	auto count = drag->getCount ();
	for (auto i = 0u; i < count; ++i)
	{
		if (drag->getDataType (i) != IDataPackage::kFilePath)
			continue;
		const void* buffer;
		IDataPackage::Type type;
		auto size = drag->getData (i, buffer, type);
		Path p (reinterpret_cast<const char*> (buffer), size);
		if (getImageSize (p))
			result.emplace_back (std::move (p));
	}
	return result;
}

//------------------------------------------------------------------------
static bool dragHasPngImages (IDataPackage* drag)
{
	auto count = drag->getCount ();
	for (auto i = 0u; i < count; ++i)
	{
		if (drag->getDataType (i) != IDataPackage::kFilePath)
			continue;
		const void* buffer;
		IDataPackage::Type type;
		auto size = drag->getData (i, buffer, type);
		Path p (reinterpret_cast<const char*> (buffer), size);
		if (getImageSize (p))
			return true;
	}
	return false;
}

//------------------------------------------------------------------------
CMouseEventResult ImageFramesView::onMouseMoved (CPoint& where, const CButtonState& buttons)
{
	if (buttons.isLeftButton ())
	{
		if (std::abs ((where.x - mouseDownPos.x) * (where.y - mouseDownPos.y)) > 5)
		{
			std::vector<size_t> indices;
			indices.emplace_back (DragPackageID);
			for (auto index = 0u; index < imageList->size (); ++index)
			{
				if (imageList->at (index).selected)
					indices.emplace_back (index);
			}
			if (indices.size () > 1)
			{
				auto dropSource = makeOwned<CDropSource> ();
				dropSource->add (indices.data (),
				                 static_cast<uint32_t> (indices.size () * sizeof (size_t)),
				                 IDataPackage::kBinary);
				DragDescription dragDesc (dropSource);
				doDrag (dragDesc);
			}
		}
		return kMouseEventHandled;
	}
	return kMouseEventNotHandled;
}

//------------------------------------------------------------------------
bool ImageFramesView::onDrop (DragEventData eventData)
{
	if (dropIndicatorPos == -1)
		return false;
	size_t dropPosition = static_cast<size_t> (dropIndicatorPos);
	std::vector<size_t> indices;
	if (getIndicesFromDataPackage (eventData.drag, &indices))
	{
		std::sort (indices.begin (), indices.end ());
		auto doCopy = (eventData.modifiers.getModifierState () & kAlt);
		std::vector<Path> paths;
		if (doCopy)
		{
			for (auto index : indices)
				paths.emplace_back (imageList->at (index).path);
			for (auto& path : paths)
			{
				docContext->insertImagePathAtIndex (dropPosition, path);
				++dropPosition;
			}
		}
		else
		{
			for (auto it = indices.rbegin (); it != indices.rend (); ++it)
			{
				auto index = *it;
				paths.emplace_back (imageList->at (index).path);
				docContext->removeImagePathAtIndex (index);
				if (index < dropPosition)
					--dropPosition;
			}
			for (auto& path : paths)
				docContext->insertImagePathAtIndex (dropPosition, path);
		}
	}
	else
	{
		auto imagePaths = getDragPngImagePaths (eventData.drag);
		if (!imagePaths.empty ())
		{
			for (auto& path : imagePaths)
			{
				docContext->insertImagePathAtIndex (dropPosition, path);
				++dropPosition;
			}
		}
	}

	dropIndicatorPos = -1;
	dragHasImages = false;
	return true;
}

//------------------------------------------------------------------------
DragOperation ImageFramesView::onDragEnter (DragEventData eventData)
{
	if (getIndicesFromDataPackage (eventData.drag))
		return DragOperation::Move;
	else if (dragHasPngImages (eventData.drag))
	{
		dragHasImages = true;
		return DragOperation::Copy;
	}
	return DragOperation::None;
}

//------------------------------------------------------------------------
void ImageFramesView::onDragLeave (DragEventData eventData)
{
	dropIndicatorPos = -1;
	dragHasImages = false;
	invalid ();
}

//------------------------------------------------------------------------
DragOperation ImageFramesView::onDragMove (DragEventData eventData)
{
	if (dragHasImages || getIndicesFromDataPackage (eventData.drag))
	{
		eventData.pos.offset (0, rowHeight / 2);
		auto newIndex = posToIndex (eventData.pos);
		if (newIndex != dropIndicatorPos)
		{
			dropIndicatorPos = newIndex;
			invalid ();
		}
		auto doCopy = dragHasImages ? true : (eventData.modifiers.getModifierState () & kAlt);
		return doCopy ? DragOperation::Copy : DragOperation::Move;
	}
	return DragOperation::None;
}

//------------------------------------------------------------------------
} // ImageStitcher
} // VSTGUI
