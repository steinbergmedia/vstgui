// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include <cairo/cairo.h>

#include "../../cpoint.h"
#include "../../vstguidebug.h"
#include "../iplatformbitmap.h"
#include "../platformfwd.h"
#include "cairoutils.h"
#include <functional>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Cairo {

//-----------------------------------------------------------------------------
class Bitmap : public IPlatformBitmap
{
public:
	static SharedPointer<Bitmap> create (UTF8StringPtr absolutePath);
	static SharedPointer<Bitmap> create (const void* ptr, uint32_t memSize);

	Bitmap ();
	explicit Bitmap (const CPoint& size);
	explicit Bitmap (const SurfaceHandle& surface);
	~Bitmap () override;

	bool load (const CResourceDescription& desc);
	const CPoint& getSize () const override;
	SharedPointer<IPlatformBitmapPixelAccess> lockPixels (bool alphaPremultiplied) override;
	void setScaleFactor (double factor) override;
	double getScaleFactor () const override;

	PNGBitmapBuffer createMemoryPNGRepresentation () const;

	const SurfaceHandle& getSurface () const
	{
		vstgui_assert (!locked, "Bitmap is locked");
		if (locked)
		{
			static SurfaceHandle empty;
			return empty;
		}
		return surface;
	}

	void unlock () { locked = false; }

private:
	double scaleFactor {1.0};
	SurfaceHandle surface;
	CPoint size;
	bool locked {false};
};

//------------------------------------------------------------------------
} // Cairo
} // VSTGUI
