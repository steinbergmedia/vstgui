// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include <cairo/cairo.h>

#include "../../cpoint.h"
#include "../../vstguidebug.h"
#include "../iplatformbitmap.h"
#include "cairoutils.h"
#include <functional>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Cairo {

//-----------------------------------------------------------------------------
class Bitmap : public IPlatformBitmap
{
public:
	explicit Bitmap (const CPoint* size);
	explicit Bitmap (const SurfaceHandle& surface);
	~Bitmap () override;

	bool load (const CResourceDescription& desc) override;
	const CPoint& getSize () const override;
	SharedPointer<IPlatformBitmapPixelAccess> lockPixels (bool alphaPremultiplied) override;
	void setScaleFactor (double factor) override;
	double getScaleFactor () const override;

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

	using GetResourcePathFunc = std::function<std::string ()>;
	static void setGetResourcePathFunc (GetResourcePathFunc&& func);

private:
	double scaleFactor {1.0};
	SurfaceHandle surface;
	CPoint size;
	bool locked {false};

	static GetResourcePathFunc getResourcePath;
};

//------------------------------------------------------------------------
} // Cairo
} // VSTGUI
