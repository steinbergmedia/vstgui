// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include <cstdint>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace ExternalView {

//------------------------------------------------------------------------
enum class PlatformViewType : uint32_t
{
	HWND,
	NSView,

	Unknown
};

//------------------------------------------------------------------------
struct IntPoint
{
	int64_t x {0};
	int64_t y {0};
};

//------------------------------------------------------------------------
struct IntSize
{
	int64_t width {0};
	int64_t height {0};
};

//------------------------------------------------------------------------
struct IntRect
{
	IntPoint origin;
	IntSize size;
};

//------------------------------------------------------------------------
struct IView
{
	virtual ~IView () noexcept = default;

	virtual bool platformViewTypeSupported (PlatformViewType type) = 0;
	virtual bool attach (void* parent, PlatformViewType parentViewType) = 0;
	virtual bool remove () = 0;

	virtual void setViewSize (IntRect frame, IntRect visible) = 0;
	virtual void setContentScaleFactor (double scaleFactor) = 0;

	virtual void setMouseEnabled (bool state) = 0;

	virtual void takeFocus () = 0;
	virtual void looseFocus () = 0;
};

//------------------------------------------------------------------------
} // ExternalView
} // VSTGUI
