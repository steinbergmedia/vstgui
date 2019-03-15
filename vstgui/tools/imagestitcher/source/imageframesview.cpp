// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "imageframesview.h"
#include "vstgui/lib/cdrawcontext.h"
#include "vstgui/lib/cdropsource.h"
#include "vstgui/lib/cframe.h"
#include "vstgui/lib/cscrollview.h"
#include "vstgui/lib/dragging.h"
#include "vstgui/standalone/include/ialertbox.h"
#include "vstgui/standalone/include/iapplication.h"
#include "vstgui/standalone/include/iasync.h"
#include <cassert>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace ImageStitcher {

using namespace VSTGUI::Standalone;

//------------------------------------------------------------------------
ImageFramesView::ImageFramesView () : CView (CRect (0, 0, 10, 10))
{
	font = makeOwned<CFontDesc> (*kSystemFont);
	font->setSize (8);
	setSelectionColor (MakeCColor (164, 205, 255, 255));
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
void ImageFramesView::makeRectVisible (CRect r) const
{
	if (!isAttached ())
		return;
	if (auto scrollView = dynamic_cast<CScrollView*> (getParentView ()->getParentView ()))
	{
		scrollView->makeRectVisible (r);
	}
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
void ImageFramesView::setSelectionColor (CColor color)
{
	activeSelectionColor = inactiveSelectionColor = color;
	double h, s, l;
	inactiveSelectionColor.toHSL (h, s, l);
	s = 0.;
	inactiveSelectionColor.fromHSL (h, s, l);
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

	context->setFillColor (getFrame ()->getFocusView () == this ? activeSelectionColor :
	                                                              inactiveSelectionColor);

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
void ImageFramesView::takeFocus ()
{
	CView::takeFocus ();
}

//------------------------------------------------------------------------
void ImageFramesView::looseFocus ()
{
	CView::looseFocus ();
}

//------------------------------------------------------------------------
int32_t ImageFramesView::firstSelectedIndex () const
{
	for (auto index = 0u; index < imageList->size (); ++index)
	{
		if (imageList->at (index).selected)
			return static_cast<int32_t> (index);
	}
	return -1;
}

//------------------------------------------------------------------------
int32_t ImageFramesView::lastSelectedIndex () const
{
	for (int32_t index = static_cast<int32_t> (imageList->size ()) - 1; index >= 0; --index)
	{
		if (imageList->at (index).selected)
			return index;
	}
	return -1;
}

//------------------------------------------------------------------------
CPoint ImageFramesView::sizeOfOneRow () const
{
	CPoint size;
	size.x = getWidth ();
	size.y = rowHeight;
	return size;
}

//------------------------------------------------------------------------
CRect ImageFramesView::indexToRect (size_t index) const
{
	CRect r;
	if (imageList->empty ())
		return r;
	auto size = sizeOfOneRow ();
	r.setSize (size);
	r.offset (0, size.y * index);
	r.offset (getViewSize ().getTopLeft ());
	return r;
}

//------------------------------------------------------------------------
int32_t ImageFramesView::posToIndex (CPoint where) const
{
	if (imageList->empty ())
		return 0;
	CRect r = indexToRect (0);
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
		if (image.selected != (i == index))
		{
			image.selected = (i == index);
			invalidRect (indexToRect (i));
		}
	}
	makeRectVisible (indexToRect (index));
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
		{
			if (!imageList->at (i).selected)
			{
				imageList->at (i).selected = true;
				invalidRect (indexToRect (i));
			}
		}
	}
	else if (nearest > 0)
	{
		for (auto i = index; i < index + nearest; ++i)
		{
			if (!imageList->at (i).selected)
			{
				imageList->at (i).selected = true;
				invalidRect (indexToRect (i));
			}
		}
	}
}

static constexpr size_t DragPackageID = 'isdp';

//------------------------------------------------------------------------
bool ImageFramesView::getIndicesFromDataPackage (IDataPackage* package, std::vector<size_t>* result)
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
std::vector<Path> ImageFramesView::getDragPngImagePaths (IDataPackage* drag)
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
bool ImageFramesView::dragHasPngImages (IDataPackage* drag)
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
void ImageFramesView::reorderImages (size_t position, bool doCopy, std::vector<size_t>& indices)
{
	std::sort (indices.begin (), indices.end ());
	std::vector<Path> paths;
	if (doCopy)
	{
		for (auto index : indices)
			paths.emplace_back (imageList->at (index).path);
		for (auto& path : paths)
		{
			docContext->insertImagePathAtIndex (position, path);
			++position;
		}
	}
	else
	{
		for (auto it = indices.rbegin (); it != indices.rend (); ++it)
		{
			auto index = *it;
			paths.emplace_back (imageList->at (index).path);
			docContext->removeImagePathAtIndex (index);
			if (index < position)
				--position;
		}
		for (auto& path : paths)
			docContext->insertImagePathAtIndex (position, path);
	}
}

//------------------------------------------------------------------------
void ImageFramesView::addImages (size_t position, const std::vector<std::string>& imagePaths)
{
	std::string alertDescription;
	for (auto& path : imagePaths)
	{
		auto res = docContext->insertImagePathAtIndex (position, path);
		switch (res)
		{
			case DocumentContextResult::Success:
			{
				++position;
				break;
			}
			case DocumentContextResult::ImageSizeMismatch:
			{
				alertDescription += "Image Size Mismatch :";
				alertDescription += path;
				alertDescription += "\n";
				break;
			}
			case DocumentContextResult::InvalidImage:
			case DocumentContextResult::InvalidIndex:
			{
				alertDescription += "Unexpected Error\n";
				break;
			}
		}
	}
	if (!alertDescription.empty ())
	{
		Async::schedule (Async::mainQueue (), [alertDescription] () {
			AlertBoxConfig alert;
			alert.headline = "Error adding images!";
			alert.description = alertDescription;
			IApplication::instance ().showAlertBox (alert);
		});
	}
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
				invalidRect (indexToRect (index));
			}
		}
		return kMouseEventHandled;
	}
	return kMouseEventNotHandled;
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
		auto doCopy = (eventData.modifiers.getModifierState () & kAlt) != 0;
		reorderImages (dropPosition, doCopy, indices);
	}
	else
	{
		auto imagePaths = getDragPngImagePaths (eventData.drag);
		if (!imagePaths.empty ())
		{
			addImages (dropPosition, imagePaths);
		}
	}

	invalid ();
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
int32_t ImageFramesView::onKeyDown (VstKeyCode& keyCode)
{
	if (keyCode.virt == 0 || !imageList || imageList->empty ())
		return -1;
	switch (keyCode.virt)
	{
		case VKEY_UP:
		{
			auto index = firstSelectedIndex ();
			if (index <= 0)
				index = static_cast<int32_t> (imageList->size ());
			--index;
			selectExclusive (static_cast<size_t> (index));
			return 1;
		}
		case VKEY_DOWN:
		{
			auto index = lastSelectedIndex ();
			if (index == static_cast<int32_t> (imageList->size ()) - 1)
				index = 0;
			else
				++index;
			selectExclusive (static_cast<size_t> (index));
			return 1;
		}
	}
	return -1;
}

//------------------------------------------------------------------------
} // ImageStitcher
} // VSTGUI
