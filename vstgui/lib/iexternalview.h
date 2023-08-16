// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include <cstdint>
#include <functional>

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
 *	@ingroup new_in_4_13
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

	/** the view should take focus  */
	virtual void takeFocus () = 0;
	/** the view should loose focus  */
	virtual void looseFocus () = 0;

	using TookFocusCallback = std::function<void ()>;
	/** a callback the embedder sets on the view to get notified when the view took focus */
	virtual void setTookFocusCallback (const TookFocusCallback& callback) = 0;
};

//------------------------------------------------------------------------
struct IControlViewExtension
{
	using ValueBeginEditCallback = std::function<void ()>;
	using ValueEndEditCallback = std::function<void ()>;
	using ValuePerformEditCallback = std::function<void (double newValue)>;
	struct EditCallbacks
	{
		ValueBeginEditCallback beginEdit;
		ValuePerformEditCallback performEdit;
		ValueEndEditCallback endEdit;
	};

	virtual bool setValue (double value) = 0;
	virtual bool setEditCallbacks (const EditCallbacks& callbacks) = 0;
};

//------------------------------------------------------------------------
/** interface for view embedder classes
 *
 *	@ingroup new_in_4_13
 */
struct IViewEmbedder
{
	virtual ~IViewEmbedder () noexcept = default;

	/** returns the embedded view or nullptr if it has none */
	virtual IView* getExternalView () const = 0;
};

//------------------------------------------------------------------------
/** adapter for the IView interface
 *
 *	@ingroup new_in_4_13
 */
struct ViewAdapter : IView
{
	bool platformViewTypeSupported (PlatformViewType type) override { return false; }
	bool attach (void* parent, PlatformViewType parentViewType) override { return false; }
	bool remove () override { return false; }
	void setViewSize (IntRect frame, IntRect visible) override {}
	void setContentScaleFactor (double scaleFactor) override {}
	void setMouseEnabled (bool state) override {}
	void takeFocus () override {}
	void looseFocus () override {}
	void setTookFocusCallback (const TookFocusCallback& callback) override {}
};

//------------------------------------------------------------------------
struct ControlViewAdapter : ViewAdapter,
							IControlViewExtension
{
	bool setValue (double value) override { return false; }
	bool setEditCallbacks (const EditCallbacks& callbacks) override { return false; }
};

//------------------------------------------------------------------------
} // ExternalView
} // VSTGUI
