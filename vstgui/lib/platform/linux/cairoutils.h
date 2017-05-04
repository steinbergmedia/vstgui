// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include <cairo/cairo.h>
#include <utility>

//-----------------------------------------------------------------------------
namespace VSTGUI {
namespace Cairo {

//-----------------------------------------------------------------------------
template <typename Type, typename RetainProcType, RetainProcType RetainProc,
		  typename ReleaseProcType, ReleaseProcType ReleaseProc>
class Handle
{
public:
	Handle () {}
	explicit Handle (Type h) : handle (h) {}
	~Handle () { reset (); }
	Handle (Handle&& o) { *this = std::move (o); }
	Handle& operator= (Handle&& o)
	{
		reset ();
		std::swap (handle, o.handle);
		return *this;
	}

	Handle (const Handle& o) { *this = o; }
	Handle& operator= (const Handle& o)
	{
		reset ();
		if (o.handle)
		{
			handle = RetainProc (o.handle);
		}
		return *this;
	}

	void assign (Type h)
	{
		reset ();
		handle = h;
	}

	void reset ()
	{
		if (handle)
		{
			ReleaseProc (handle);
			handle = nullptr;
		}
	}

	operator Type () const { return handle; }
	operator bool () const { return handle != nullptr; }

private:
	Type handle {nullptr};
};

using ContextHandle = Handle<cairo_t*, decltype (&cairo_reference), cairo_reference,
							 decltype (&cairo_destroy), cairo_destroy>;

using SurfaceHandle =
	Handle<cairo_surface_t*, decltype (&cairo_surface_reference), cairo_surface_reference,
		   decltype (&cairo_surface_destroy), cairo_surface_destroy>;

using PatternHandle =
	Handle<cairo_pattern_t*, decltype (&cairo_pattern_reference), cairo_pattern_reference,
		   decltype (&cairo_pattern_destroy), cairo_pattern_destroy>;

using ScaledFontHandle = Handle<cairo_scaled_font_t*, decltype (&cairo_scaled_font_reference),
								cairo_scaled_font_reference, decltype (&cairo_scaled_font_destroy),
								cairo_scaled_font_destroy>;

//-----------------------------------------------------------------------------
} // Cairo
} // VSTGUI
