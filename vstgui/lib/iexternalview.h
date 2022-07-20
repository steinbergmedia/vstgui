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
/** interface for embedding views from external view systems
 *
 *	@ingroup new_in_4_12
 */
struct IView
{
	virtual ~IView () noexcept = default;

	/** check if the view supports the platform view type */
	virtual bool platformViewTypeSupported (PlatformViewType type) = 0;
	/** attach the view to the parent */
	virtual bool attach (void* parent, PlatformViewType parentViewType) = 0;
	/** remove the view from its parent */
	virtual bool remove () = 0;

	/** set the size and position of the view
	 *
	 *	the coordinate system for the parameters used are system native,
	 *	this means on Windows the scale factor is already applied to the coordinates
	 *	and on macOS they are not (as with all NSViews)
	 *
	 *	the visible rectangle is the rectangle clipped to all its parent views
	 *	while the frame rectangle is the full size of the view if it would not
	 *	be clipped. this way it is possible to support views inside of scroll views
	 *	and the like. See the examples how this is done.
	 */
	virtual void setViewSize (IntRect frame, IntRect visible) = 0;
	/** set the scale factor in use */
	virtual void setContentScaleFactor (double scaleFactor) = 0;

	/** enable or disable mouse handling for the view */
	virtual void setMouseEnabled (bool state) = 0;

	/** TODO: */
	virtual void takeFocus () = 0;
	/** TODO: */
	virtual void looseFocus () = 0;
};

//------------------------------------------------------------------------
struct IViewEmbedder
{
	virtual ~IViewEmbedder () noexcept = default;

	/** returns the embedded view or nullptr if it has none */
	virtual IView* getExternalView () const = 0;
};

//------------------------------------------------------------------------
} // ExternalView
} // VSTGUI
