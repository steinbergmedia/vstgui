// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "../../cpoint.h"
#include "../../cresourcedescription.h"

#include "cairobitmap.h"
#include "x11platform.h"
#include <memory>
#include <vector>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Cairo {
namespace CairoBitmapPrivate {

//-----------------------------------------------------------------------------
struct PNGMemoryReader
{
	PNGMemoryReader (const uint8_t* ptr, size_t size) : ptr (ptr), size (size) {}

	cairo_surface_t* create () { return cairo_image_surface_create_from_png_stream (read, this); }

private:
	static cairo_status_t read (void* closure, unsigned char* data, unsigned int length)
	{
		auto self = reinterpret_cast<PNGMemoryReader*> (closure);
		auto numBytes = std::min<size_t> (length, self->size);
		if (numBytes)
		{
			memcpy (data, self->ptr, numBytes);
			self->ptr += numBytes;
			self->size -= numBytes;
			return CAIRO_STATUS_SUCCESS;
		}
		return CAIRO_STATUS_READ_ERROR;
	}

	const uint8_t* ptr;
	size_t size;
};

//-----------------------------------------------------------------------------
struct PNGMemoryWriter
{
	using Buffer = PNGBitmapBuffer;

	Buffer create (cairo_surface_t* image)
	{
		Buffer buffer;
		cairo_surface_write_to_png_stream (image, write, &buffer);
		return buffer;
	}

private:
	static cairo_status_t write (void* closure, const unsigned char* data, unsigned int length)
	{
		auto buffer = reinterpret_cast<Buffer*> (closure);
		if (!buffer)
			return CAIRO_STATUS_WRITE_ERROR;
		buffer->reserve (buffer->size () + length);
		std::copy_n (data, length, std::back_inserter (*buffer));
		return CAIRO_STATUS_SUCCESS;
	}
};

//-----------------------------------------------------------------------------
static SurfaceHandle createImageFromPath (const char* path)
{
	if (auto surface = cairo_image_surface_create_from_png (path))
	{
		if (cairo_surface_status (surface) != CAIRO_STATUS_SUCCESS)
		{
			cairo_surface_destroy (surface);
			return {};
		}
		if (cairo_image_surface_get_format (surface) == CAIRO_FORMAT_ARGB32)
			return SurfaceHandle {surface};

		// vstgui always works with 32 bit images
		auto x = cairo_image_surface_get_width (surface);
		auto y = cairo_image_surface_get_height (surface);
		auto surface32 = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, x, y);
		vstgui_assert (cairo_surface_status (surface32) == CAIRO_STATUS_SUCCESS);
		auto context = cairo_create (surface32);
		vstgui_assert (cairo_status (context) == CAIRO_STATUS_SUCCESS);
		cairo_set_source_surface (context, surface, 0, 0);
		vstgui_assert (cairo_status (context) == CAIRO_STATUS_SUCCESS);
		cairo_paint (context);
		vstgui_assert (cairo_status (context) == CAIRO_STATUS_SUCCESS);
		cairo_surface_flush (surface32);
		vstgui_assert (cairo_status (context) == CAIRO_STATUS_SUCCESS);
		cairo_destroy (context);
		cairo_surface_destroy (surface);
		return SurfaceHandle {surface32};
	}
	return {};
}

//-----------------------------------------------------------------------------
class PixelAccess : public IPlatformBitmapPixelAccess
{
public:
	~PixelAccess () override;

	bool init (Bitmap* bitmap, const SurfaceHandle& surface);

private:
	uint8_t* address {nullptr};
	uint32_t bytesPerRow {0};

	uint8_t* getAddress () const override { return address; }
	uint32_t getBytesPerRow () const override { return bytesPerRow; }
	PixelFormat getPixelFormat () const override
	{
#if __LITTLE_ENDIAN
		return kBGRA;
#else
		return kARGB;
#endif
	}

	SharedPointer<Bitmap> bitmap;
	SurfaceHandle surface;
};

//-----------------------------------------------------------------------------
} // CairoBitmapPrivate

//-----------------------------------------------------------------------------
Bitmap::Bitmap (CPoint* _size)
{
	if (_size)
	{
		size = *_size;
		surface = SurfaceHandle (cairo_image_surface_create (CAIRO_FORMAT_ARGB32, size.x, size.y));
	}
}

//-----------------------------------------------------------------------------
Bitmap::Bitmap (const SurfaceHandle& surface) : surface (surface)
{
	size.x = cairo_image_surface_get_width (surface);
	size.y = cairo_image_surface_get_height (surface);
}

//-----------------------------------------------------------------------------
Bitmap::~Bitmap ()
{
}

//-----------------------------------------------------------------------------
bool Bitmap::load (const CResourceDescription& desc)
{
	auto path = X11::Platform::getInstance ().getPath ();
	if (!path.empty ())
	{
		path += "/Contents/Resources/";
		if (desc.type == CResourceDescription::kIntegerType)
		{
			char filename[PATH_MAX];
			sprintf (filename, "bmp%05d.png", (int32_t)desc.u.id);
			path += filename;
		}
		else
		{
			path += desc.u.name;
		}
		if (auto s = CairoBitmapPrivate::createImageFromPath (path.data ()))
		{
			if (cairo_surface_status (s) != CAIRO_STATUS_SUCCESS)
			{
				cairo_surface_destroy (s);
				return false;
			}
			surface = s;
			size.x = cairo_image_surface_get_width (surface);
			size.y = cairo_image_surface_get_height (surface);
			return true;
		}
	}
	return false;
}

//-----------------------------------------------------------------------------
const CPoint& Bitmap::getSize () const
{
	return size;
}
//-----------------------------------------------------------------------------
SharedPointer<IPlatformBitmapPixelAccess> Bitmap::lockPixels (bool alphaPremultiplied)
{
	if (locked)
		return nullptr;
#warning TODO: alphaPremultiplied is currently ignored, always treated as true
	locked = true;
	auto pixelAccess = owned (new CairoBitmapPrivate::PixelAccess ());
	if (pixelAccess->init (this, surface))
		return pixelAccess;
	return nullptr;
}

//-----------------------------------------------------------------------------
void Bitmap::setScaleFactor (double factor)
{
	scaleFactor = factor;
}

//-----------------------------------------------------------------------------
double Bitmap::getScaleFactor () const
{
	return scaleFactor;
}

//-----------------------------------------------------------------------------
namespace CairoBitmapPrivate {

//-----------------------------------------------------------------------------
bool PixelAccess::init (Bitmap* inBitmap, const SurfaceHandle& inSurface)
{
	cairo_surface_flush (inSurface);
	address = cairo_image_surface_get_data (inSurface);
	if (!address)
	{
#if DEBUG
		auto status = cairo_surface_status (inSurface);
		if (status != CAIRO_STATUS_SUCCESS)
		{
			auto msg = cairo_status_to_string (status);
			DebugPrint ("%s\n", msg);
		}
#endif
		return false;
	}
	surface = inSurface;
	bitmap = inBitmap;
	bytesPerRow = cairo_image_surface_get_stride (surface);
	return true;
}

//-----------------------------------------------------------------------------
PixelAccess::~PixelAccess ()
{
	cairo_surface_mark_dirty (surface);
	bitmap->unlock ();
}

//-----------------------------------------------------------------------------
} // CairoBitmapPrivate
} // Cairo

//-----------------------------------------------------------------------------
SharedPointer<IPlatformBitmap> IPlatformBitmap::create (CPoint* size)
{
	return owned (new Cairo::Bitmap (size));
}

//-----------------------------------------------------------------------------
SharedPointer<IPlatformBitmap> IPlatformBitmap::createFromPath (UTF8StringPtr absolutePath)
{
	if (auto surface = Cairo::CairoBitmapPrivate::createImageFromPath (absolutePath))
	{
		if (cairo_surface_status (surface) != CAIRO_STATUS_SUCCESS)
		{
			cairo_surface_destroy (surface);
			return nullptr;
		}
		return owned (new Cairo::Bitmap (surface));
	}
	return nullptr;
}

//-----------------------------------------------------------------------------
SharedPointer<IPlatformBitmap> IPlatformBitmap::createFromMemory (const void* ptr, uint32_t memSize)
{
	Cairo::CairoBitmapPrivate::PNGMemoryReader reader (reinterpret_cast<const uint8_t*> (ptr),
													   memSize);
	if (auto surface = reader.create ())
	{
		return owned (new Cairo::Bitmap (Cairo::SurfaceHandle {surface}));
	}
	return nullptr;
}

//-----------------------------------------------------------------------------
PNGBitmapBuffer IPlatformBitmap::createMemoryPNGRepresentation (const SharedPointer<IPlatformBitmap>& bitmap)
{
	if (auto cairoBitmap = bitmap.cast<Cairo::Bitmap> ())
	{
		Cairo::CairoBitmapPrivate::PNGMemoryWriter writer;
		return writer.create (cairoBitmap->getSurface ());
	}
	return {};
}

//-----------------------------------------------------------------------------
} // VSTGUI
