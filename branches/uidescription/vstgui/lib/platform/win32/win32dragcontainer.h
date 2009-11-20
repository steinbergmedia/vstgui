
#ifndef __win32dragcontainer__
#define __win32dragcontainer__

#include "../../cview.h"

#if WINDOWS

#include <windows.h>

namespace VSTGUI {

class WinDragContainer : public CDragContainer
{
public:
	WinDragContainer (IDataObject* platformDrag);
	~WinDragContainer ();

	virtual void* first (long& size, long& type);		///< returns pointer on a char array if type is known
	virtual void* next (long& size, long& type);		///< returns pointer on a char array if type is known
	
	virtual long getType (long idx) const;
	virtual long getCount () const { return nbItems; }

protected:
	static bool checkResolveLink (const TCHAR* nativePath, TCHAR* resolved);

	IDataObject* platformDrag;
	long nbItems;
	
	long iterator;
	void* lastItem;
};

} // namespace

#endif // WINDOWS

#endif // __win32dragcontainer__