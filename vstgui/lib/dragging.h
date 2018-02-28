#pragma once

#include "vstguifwd.h"
#include "cbitmap.h"
#include "cpoint.h"
#include "idatapackage.h"

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
} // VSTGUI
