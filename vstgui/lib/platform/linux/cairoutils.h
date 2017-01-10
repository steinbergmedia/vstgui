//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 4.3
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2015, Steinberg Media Technologies, All Rights Reserved
//-----------------------------------------------------------------------------
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of the Steinberg Media Technologies nor the names of its
//     contributors may be used to endorse or promote products derived from this
//     software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

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
