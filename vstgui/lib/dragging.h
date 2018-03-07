#pragma once

#include "vstguifwd.h"
#include "cbitmap.h"
#include "cpoint.h"
#include "idatapackage.h"
#include <functional>

//------------------------------------------------------------------------
namespace VSTGUI {

//-----------------------------------------------------------------------------
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
class IDraggingSession
{
public:
	virtual bool setBitmap (const SharedPointer<CBitmap>& bitmap, CPoint offset) = 0;
};

//------------------------------------------------------------------------
class IDragCallback : virtual public IReference
{
public:
	/** Indicates that a drag will begin
	 *	@param session dragging session
	 *	@param pos drag position in cframe coordinates
	 */
	virtual void dragWillBegin (IDraggingSession* session, CPoint pos) = 0;
	/** The drag was moved
	 *	@param session dragging session
	 *	@param pos drag position in cframe coordinates
	 */
	virtual void dragMoved (IDraggingSession* session, CPoint pos) = 0;
	/** The drag ended
	 *	@param session dragging session
	 *	@param pos drag position in cframe coordinates
	 *	@param result the result of the drag
	 */
	virtual void dragEnded (IDraggingSession* session, CPoint pos, DragResult result) = 0;
};

//------------------------------------------------------------------------
class DragCallbackAdapter : virtual public IDragCallback
{
public:
	void dragWillBegin (IDraggingSession* session, CPoint pos) override {}
	void dragMoved (IDraggingSession* session, CPoint pos) override {}
	void dragEnded (IDraggingSession* session, CPoint pos, DragResult result) override {}
};

//------------------------------------------------------------------------
class DragCallbackFunctions : virtual public IDragCallback, public NonAtomicReferenceCounted
{
public:
	using Func1 = std::function<void (IDraggingSession*, CPoint)>;
	using Func2 = std::function<void (IDraggingSession*, CPoint, DragResult)>;

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
	void dragEnded (IDraggingSession* session, CPoint pos, DragResult result) override
	{
		if (endedFunc)
			endedFunc (session, pos, result);
	}

	Func1 willBeginFunc;
	Func1 movedFunc;
	Func2 endedFunc;
};

//------------------------------------------------------------------------
} // VSTGUI
