#ifndef __cgbitmap__
#define __cgbitmap__

#include "../iplatformbitmap.h"

#if MAC

#include <ApplicationServices/ApplicationServices.h>

BEGIN_NAMESPACE_VSTGUI

//-----------------------------------------------------------------------------
class CGBitmap : public IPlatformBitmap
{
public:
	CGBitmap ();
	~CGBitmap ();
	
	bool load (const CResourceDescription& desc);
	const CPoint& getSize () const { return size; }

	bool loadFromUrl (CFURLRef url);
	void loadImage ();
	virtual CGImageRef getCGImage ();

//-----------------------------------------------------------------------------
protected:
	CPoint size;
	CGImageRef image;
	CGImageSourceRef imageSource;
};

//-----------------------------------------------------------------------------
class CGOffscreenBitmap : public CGBitmap
{
public:
	CGOffscreenBitmap (const CPoint& size);
	~CGOffscreenBitmap ();
	
	bool load (const CResourceDescription& desc) { return false; }

	CGImageRef getCGImage ();
	CGContextRef createCGContext ();
	void setDirty () { dirty = true; }
	void* getBits () const { return bits; }
	int getBytesPerRow () const;

//-----------------------------------------------------------------------------
protected:
	void allocBits ();
	
	void* bits;
	bool dirty;
};

END_NAMESPACE_VSTGUI

#endif // MAC
#endif // __cgbitmap__
