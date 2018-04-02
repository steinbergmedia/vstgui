// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "documentcontroller.h"
#include "vstgui/lib/cbitmap.h"
#include "vstgui/lib/ccolor.h"
#include "vstgui/lib/cfont.h"
#include "vstgui/lib/cview.h"
#include "vstgui/lib/dragging.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace ImageStitcher {

//------------------------------------------------------------------------
class ImageFramesView : public CView, public IDropTarget
{
public:
	ImageFramesView ();

	void setDocContext (const DocumentContextPtr& dc);
	void setImageList (ImageList* list);

private:
	int32_t posToIndex (CPoint where) const;
	void selectExclusive (size_t index);
	void enlargeSelection (size_t index);
	void updateViewSize ();
	void parentSizeChanged () override;
	void drawRect (CDrawContext* context, const CRect& _updateRect) override;
	CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons) override;
	SharedPointer<IDropTarget> getDropTarget () override { return this; }
	DragOperation onDragEnter (DragEventData eventData) override;
	DragOperation onDragMove (DragEventData eventData) override;
	void onDragLeave (DragEventData eventData) override;
	bool onDrop (DragEventData eventData) override;

	CColor selectionColor = MakeCColor (164, 205, 255, 255);
	CCoord titleHeight {8};
	CCoord rowHeight {0};
	CPoint mouseDownPos;
	SharedPointer<CFontDesc> font;
	DocumentContextPtr docContext;
	ImageList* imageList {nullptr};

	int32_t dropIndicatorPos {-1};
	bool dragHasImages {false};
};

//------------------------------------------------------------------------
} // ImageStitcher
} // VSTGUI
