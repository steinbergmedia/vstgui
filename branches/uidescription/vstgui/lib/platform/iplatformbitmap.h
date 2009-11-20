#ifndef __iplatformbitmap__
#define __iplatformbitmap__

#include "../cbitmap.h"

BEGIN_NAMESPACE_VSTGUI
class CResourceDescription;

//-----------------------------------------------------------------------------
class IPlatformBitmap : public CBaseObject
{
public:
	static IPlatformBitmap* create (CPoint* size = 0); // if size pointer is not zero, create a bitmap which can be used as a draw surface

	virtual bool load (const CResourceDescription& desc) = 0;
	virtual const CPoint& getSize () const = 0;
};

END_NAMESPACE_VSTGUI

#endif // __iplatformbitmap__
