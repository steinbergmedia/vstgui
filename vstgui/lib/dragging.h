// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "vstguifwd.h"
#include "cbitmap.h"
#include "events.h"
#include "cpoint.h"
#include "idatapackage.h"
#include <functional>

//------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------
/** Drag description
 *
 *	@ingroup new_in_4_7
 */
struct DragDescription
{
	SharedPointer<IDataPackage> data;
	SharedPointer<CBitmap> bitmap;
	CPoint bitmapOffset;

	explicit DragDescription (const SharedPointer<IDataPackage>& data,
	                          CPoint bitmapOffset = CPoint (),
	                          const SharedPointer<CBitmap>& bitmap = nullptr)
	: data (data), bitmap (bitmap), bitmapOffset (bitmapOffset)
	{
	}
};

//------------------------------------------------------------------------
/** Drag operation
 *
 *	@ingroup new_in_4_7
 */
enum class DragOperation
{
	/** the drag will be or was copied*/
	Copy,
	/** the drag will be or was moved*/
	Move,
	/** no drag operation will occure or was happening */
	None
};

//------------------------------------------------------------------------
/** Dragging session interface
 *
 *	@ingroup new_in_4_7
 */
class IDraggingSession
{
public:
	virtual bool setBitmap (const SharedPointer<CBitmap>& bitmap, CPoint offset) = 0;
};

//------------------------------------------------------------------------
/** Drag event data
 *
 *	@ingroup new_in_4_7
 */
struct DragEventData
{
	/** drag data package */
	IDataPackage* drag;
	/** mouse position */
	CPoint pos;
	/** key modifiers */
	Modifiers modifiers;
};

//------------------------------------------------------------------------
/** Drop target interface
 *
 *	handles drag'n drop for a view
 *
 *	The workflow is:
 *
 *	- drag enters a view, the @b getDropTarget() of the view is called
 *
 *	- the drop targets @b onDragEnter() is called
 *
 *	- the drop targets @b onDragMove() is called whenever the mouse moves inside the view
 *
 *	- when the mouse leaves the view, the drop targets @b onDragLeave() is called and its reference
 *		count is decreased.
 *
 *	- when the drag is droped, the drop targets @b onDrop() is called and then its reference count
 *		is decreased. Note that no @b onDragLeave() is called.
 *
 *	@ingroup new_in_4_7
 */
class IDropTarget : virtual public IReference
{
public:
	/** a drag enters the drop target
	 *	@param data drag event data
	 *	@return drag operation
	 */
	virtual DragOperation onDragEnter (DragEventData data) = 0;
	/** a drag moves over the drop target
	 *	@param data drag event data
	 *	@return drag operation
	 */
	virtual DragOperation onDragMove (DragEventData data) = 0;
	/** a drag leaves the drop target
	 *	@param data drag event data
	 */
	virtual void onDragLeave (DragEventData data) = 0;
	/** a drop happens on the drop target
	 *	@param data drag event data
	 */
	virtual bool onDrop (DragEventData data) = 0;
};

//------------------------------------------------------------------------
/** Drag callback interface
 *
 *	An optional interface to be used when initiating a drag to know where the mouse is and what the
 *	result of the drag was.
 *
 *	@ingroup new_in_4_7
 */
class IDragCallback : virtual public IReference
{
public:
	/** the drag will begin
	 *	@param session dragging session
	 *	@param pos drag position in CFrame coordinates
	 */
	virtual void dragWillBegin (IDraggingSession* session, CPoint pos) = 0;
	/** the drag was moved
	 *	@param session dragging session
	 *	@param pos drag position in CFrame coordinates
	 */
	virtual void dragMoved (IDraggingSession* session, CPoint pos) = 0;
	/** the drag ended
	 *	@param session dragging session
	 *	@param pos drag position in CFrame coordinates
	 *	@param result the result of the drag
	 */
	virtual void dragEnded (IDraggingSession* session, CPoint pos, DragOperation result) = 0;
};

//------------------------------------------------------------------------
/** Drag callback interface adapter
 *
 *	@ingroup new_in_4_7
 */
class DragCallbackAdapter : virtual public IDragCallback
{
public:
	void dragWillBegin (IDraggingSession* session, CPoint pos) override {}
	void dragMoved (IDraggingSession* session, CPoint pos) override {}
	void dragEnded (IDraggingSession* session, CPoint pos, DragOperation result) override {}
};

//------------------------------------------------------------------------
/** Drag callback interface adapter which calls std::functions
 *
 *	@ingroup new_in_4_7
 */
class DragCallbackFunctions : virtual public IDragCallback, public NonAtomicReferenceCounted
{
public:
	using Func1 = std::function<void (IDraggingSession*, CPoint)>;
	using Func2 = std::function<void (IDraggingSession*, CPoint, DragOperation)>;

	DragCallbackFunctions () = default;

	void dragWillBegin (IDraggingSession* session, CPoint pos) override
	{
		if (willBeginFunc)
			willBeginFunc (session, pos);
	}
	void dragMoved (IDraggingSession* session, CPoint pos) override
	{
		if (movedFunc)
			movedFunc (session, pos);
	}
	void dragEnded (IDraggingSession* session, CPoint pos, DragOperation result) override
	{
		if (endedFunc)
			endedFunc (session, pos, result);
	}

	Func1 willBeginFunc;
	Func1 movedFunc;
	Func2 endedFunc;
};

//------------------------------------------------------------------------
/** Drop target interface adapter
 *
 *	@ingroup new_in_4_7
 */
class DropTargetAdapter : virtual public IDropTarget
{
public:
	DragOperation onDragEnter (DragEventData eventData) override { return DragOperation::None; }
	DragOperation onDragMove (DragEventData eventData) override { return DragOperation::None; }
	void onDragLeave (DragEventData eventData) override {}
	bool onDrop (DragEventData eventData) override { return false; }
};

//------------------------------------------------------------------------
/** Helper method to be used to decide if a mouse move is far enough to start a drag operation.
 *	\sa DragStartMouseObserver
 *
 *	@ingroup new_in_4_9
 */
inline bool shouldStartDrag (CPoint mouseDownPos, CPoint mouseMovePos)
{
	constexpr auto minDiff = 4;
	return (std::abs (mouseDownPos.x - mouseMovePos.x) >= minDiff ||
	        std::abs (mouseDownPos.y - mouseMovePos.y) >= minDiff);
}

//------------------------------------------------------------------------
/** Helper object to be used to decide if a mouse move is far enough to start a drag operation
 *	\sa shouldStartDrag
 *
 *	@ingroup new_in_4_9
 */
struct DragStartMouseObserver
{
	void init (CPoint mousePos) { pos = mousePos; }
	bool shouldStartDrag (CPoint mousePos) const { return VSTGUI::shouldStartDrag (pos, mousePos); }

	CPoint getInitPosition () const { return pos; }
private:
	CPoint pos {};
};

//------------------------------------------------------------------------
} // VSTGUI
