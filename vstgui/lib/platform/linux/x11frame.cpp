//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 4.3
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2016, Steinberg Media Technologies, All Rights Reserved
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

#include "x11frame.h"
#include "../../cbuttonstate.h"
#include "../../crect.h"
#include "../../idatapackage.h"
#include "../../vstkeycode.h"
#include "cairobitmap.h"
#include "cairocontext.h"
#include "gtkoptionmenu.h"
#include "gtktextedit.h"
#include "x11platform.h"
#include "x11timer.h"
#include <gtkmm.h>
#include <gtkmm/plug.h>
#include <iostream>
#include <unordered_map>
#include <cassert>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace X11 {

//------------------------------------------------------------------------
namespace {

//------------------------------------------------------------------------
using VirtMap = std::unordered_map<guint, uint16_t>;
const VirtMap keyMap = {{GDK_KEY_BackSpace, VKEY_BACK},
                        {GDK_KEY_Tab, VKEY_TAB},
                        {GDK_KEY_ISO_Left_Tab, VKEY_TAB},
                        {GDK_KEY_Clear, VKEY_CLEAR},
                        {GDK_KEY_Return, VKEY_RETURN},
                        {GDK_KEY_Pause, VKEY_PAUSE},
                        {GDK_KEY_Escape, VKEY_ESCAPE},
                        {GDK_KEY_space, VKEY_SPACE},
                        {GDK_KEY_KP_Next, VKEY_NEXT},
                        {GDK_KEY_End, VKEY_END},
                        {GDK_KEY_Home, VKEY_HOME},

                        {GDK_KEY_Left, VKEY_LEFT},
                        {GDK_KEY_Up, VKEY_UP},
                        {GDK_KEY_Right, VKEY_RIGHT},
                        {GDK_KEY_Down, VKEY_DOWN},
                        {GDK_KEY_Page_Up, VKEY_PAGEUP},
                        {GDK_KEY_Page_Down, VKEY_PAGEDOWN},
                        {GDK_KEY_KP_Page_Up, VKEY_PAGEUP},
                        {GDK_KEY_KP_Page_Down, VKEY_PAGEDOWN},

                        {GDK_KEY_Select, VKEY_SELECT},
                        {GDK_KEY_Print, VKEY_PRINT},
                        {GDK_KEY_KP_Enter, VKEY_ENTER},
                        {GDK_KEY_Insert, VKEY_INSERT},
                        {GDK_KEY_Delete, VKEY_DELETE},
                        {GDK_KEY_Help, VKEY_HELP},

                        {GDK_KEY_KP_Multiply, VKEY_MULTIPLY},
                        {GDK_KEY_KP_Add, VKEY_ADD},
                        {GDK_KEY_KP_Separator, VKEY_SEPARATOR},
                        {GDK_KEY_KP_Subtract, VKEY_SUBTRACT},
                        {GDK_KEY_KP_Decimal, VKEY_DECIMAL},
                        {GDK_KEY_KP_Divide, VKEY_DIVIDE},
                        {GDK_KEY_F1, VKEY_F1},
                        {GDK_KEY_F2, VKEY_F2},
                        {GDK_KEY_F3, VKEY_F3},
                        {GDK_KEY_F4, VKEY_F4},
                        {GDK_KEY_F5, VKEY_F5},
                        {GDK_KEY_F6, VKEY_F6},
                        {GDK_KEY_F7, VKEY_F7},
                        {GDK_KEY_F8, VKEY_F8},
                        {GDK_KEY_F9, VKEY_F9},
                        {GDK_KEY_F10, VKEY_F10},
                        {GDK_KEY_F11, VKEY_F11},
                        {GDK_KEY_F12, VKEY_F12},
                        {GDK_KEY_Num_Lock, VKEY_NUMLOCK},
                        {GDK_KEY_Scroll_Lock, VKEY_SCROLL}, // correct ?
                        {GDK_KEY_Shift_L, VKEY_SHIFT},
                        {GDK_KEY_Shift_R, VKEY_SHIFT},
                        {GDK_KEY_Control_L, VKEY_CONTROL},
                        {GDK_KEY_Control_R, VKEY_CONTROL},
                        {GDK_KEY_Alt_L, VKEY_ALT},
                        {GDK_KEY_Alt_R, VKEY_ALT},
                        {GDK_KEY_VoidSymbol, 0}};

//------------------------------------------------------------------------
std::pair<CButtonState, CPoint> getDevicePosition (const Gtk::Widget& widget)
{
	auto window = widget.get_window ();
	auto device = window->get_display ()->get_device_manager ()->get_client_pointer ();
	Gdk::ModifierType mod;
	int x, y;
	window->get_device_position (device, x, y, mod);
	CButtonState buttonState {};
	if (mod & Gdk::BUTTON1_MASK)
		buttonState |= kLButton;
	if (mod & Gdk::BUTTON2_MASK)
		buttonState |= kMButton;
	if (mod & Gdk::BUTTON3_MASK)
		buttonState |= kRButton;
	return {buttonState, CPoint (x, y)};
}

//------------------------------------------------------------------------
struct GtkDragSourcePackage : public IDataPackage
{
#warning TODO: IDataPackage::kBinary support
	GtkDragSourcePackage (Gtk::Widget& widget, const Glib::RefPtr<Gdk::DragContext>& dragContext)
	: widget (widget), dragContext (dragContext)
	{
		auto dragTargets = dragContext->list_targets ();
		uint32_t uriListIndex = std::numeric_limits<uint32_t>::max ();
		uint32_t counter = 0;
		for (auto& t : dragTargets)
		{
#if DEBUG
			std::cout << t << std::endl;
#endif
			if (t == "text/plain")
				targets.emplace_back (t);
			else if (t == "plain/uri-list" || t == "text/uri-list")
			{
				uriListIndex = counter;
				targets.emplace_back (t);
			}
			++counter;
		}
		data.resize (targets.size ());
		if (uriListIndex < data.size ())
		{
			getData (uriListIndex);
			if (data[uriListIndex].buffer)
			{
				std::stringstream stream (
				    reinterpret_cast<const char*> (data[uriListIndex].buffer));
				data.clear ();
				targets.clear ();
				std::string item;
				while (std::getline (stream, item, '\n'))
				{
					if (item.find_first_of ("file://") != 0)
						continue;
					item.erase (0, 7);
					item.erase (item.size () - 1); // remove \r from end
					targets.emplace_back ("filepath");
					auto buffer = std::calloc (item.size () + 1, 1);
					std::memcpy (buffer, item.data (), item.size ());
					data.emplace_back (buffer, item.size ());
				}
			}
		}
	}

	void getData (uint32_t index) const
	{
		if (data[index].size != 0)
			return;
		CBaseObjectGuard guard (const_cast<GtkDragSourcePackage*> (this));
		LocalEventLoop eventLoop;
		bool dataReceived = false;
		auto signal = widget.signal_drag_data_received ();
		auto connection =
		    signal.connect ([&] (const Glib::RefPtr<Gdk::DragContext>& context, int, int,
		                         const Gtk::SelectionData& selectionData, guint, guint time) {
			    auto dataSize = selectionData.get_length ();
				const auto bytes = selectionData.get_data ();
				data[index].size = dataSize;
				data[index].buffer = std::calloc (dataSize + 1, 1);
				std::memcpy (data[index].buffer, bytes, dataSize);
				dataReceived = true;
				if (eventLoop.isRunning ())
					eventLoop.stop ();
			});
		widget.drag_get_data (dragContext, targets[index], gtk_get_current_event_time ());
		if (!dataReceived)
			eventLoop.run ();
		connection.disconnect ();
	}

	uint32_t getCount () const override { return targets.size (); }

	uint32_t getDataSize (uint32_t index) const override
	{
		if (index >= getCount ())
			return 0;
		auto type = getDataType (index);
		if (type == kError)
			return 0;
		getData (index);
		return data[index].size;
	}

	Type getDataType (uint32_t index) const override
	{
		if (index >= getCount ())
			return kError;
		const auto& target = targets[index];
		if (target == "text/plain")
			return kText;
		else if (target == "filepath")
			return kFilePath;
		return kError;
	}
	uint32_t getData (uint32_t index, const void*& buffer, Type& type) const override
	{
		type = getDataType (index);
		if (type == kError)
			return 0;
		getData (index);
		if (data[index].size > 0)
		{
			buffer = data[index].buffer;
			return data[index].size;
		}
		return kError;
	}

	Gtk::Widget& widget;
	Glib::RefPtr<Gdk::DragContext> dragContext;
	std::vector<std::string> targets;

	struct Entry
	{
		Entry () noexcept = default;
		Entry (void* buffer, uint32_t size) noexcept : buffer (buffer), size (size) {}
		~Entry () noexcept
		{
			if (buffer)
				std::free (buffer);
		}
		Entry (const Entry&) = delete;
		Entry (Entry&&) = default;
		Entry& operator= (const Entry&) = delete;
		Entry& operator= (Entry&) = default;

		void* buffer {nullptr};
		uint32_t size {0};
	};
	mutable std::vector<Entry> data;
};

//------------------------------------------------------------------------
} // anonymous

//------------------------------------------------------------------------
VstKeyCode keyCodeFromEvent (GdkEventKey* event)
{
	VstKeyCode key {};
	if (event->state & GDK_SHIFT_MASK)
		key.modifier |= MODIFIER_SHIFT;
	if (event->state & GDK_CONTROL_MASK)
		key.modifier |= MODIFIER_CONTROL;
	if (event->state & GDK_MOD1_MASK)
		key.modifier |= MODIFIER_ALTERNATE;
	auto it = keyMap.find (event->keyval);
	if (it != keyMap.end ())
		key.virt = it->second;
	else
		key.character = towlower (gdk_keyval_to_unicode (event->keyval));
	return key;
}

//------------------------------------------------------------------------
struct GtkFrame : Gtk::DrawingArea
{
	IPlatformFrameCallback* frame {nullptr};

	GtkFrame ()
	{
		using namespace Gdk;
		EventMask eventMask {};
		eventMask |= BUTTON_PRESS_MASK;
		eventMask |= BUTTON_RELEASE_MASK;
		eventMask |= POINTER_MOTION_MASK;
		eventMask |= SCROLL_MASK;
		eventMask |= SMOOTH_SCROLL_MASK;
		eventMask |= KEY_PRESS_MASK;
		eventMask |= KEY_RELEASE_MASK;
		eventMask |= FOCUS_CHANGE_MASK;
		eventMask |= STRUCTURE_MASK;
		set_events (eventMask);
		set_can_focus (true);

		std::vector<Gtk::TargetEntry> supportedDragTypes;
		supportedDragTypes.emplace_back ("text/plain");
		supportedDragTypes.emplace_back ("text/uri-list");
		drag_dest_set (supportedDragTypes, Gtk::DEST_DEFAULT_MOTION,
		               Gdk::ACTION_COPY | Gdk::ACTION_MOVE);
	}

	void on_size_allocate (Gtk::Allocation& allocation) override
	{
		allocation.set_width (size.getWidth ());
		allocation.set_height (size.getHeight ());
		Gtk::DrawingArea::on_size_allocate (allocation);
	}

	bool on_draw (const ::Cairo::RefPtr<::Cairo::Context>& cr) override
	{
		if (!frame)
			return false;

		inDraw = true;
		if (auto dirtyRects = cairo_copy_clip_rectangle_list (cr->cobj ()))
		{
			auto allocation = get_allocation ();
			CRect size;
			size.setWidth (allocation.get_width ());
			size.setHeight (allocation.get_height ());
			Cairo::Context drawContext (size, cr->cobj ());
			drawContext.beginDraw ();
			for (auto i = 0; i < dirtyRects->num_rectangles; ++i)
			{
				CRect dr;
				dr.left = dirtyRects->rectangles[i].x;
				dr.top = dirtyRects->rectangles[i].y;
				dr.setWidth (dirtyRects->rectangles[i].width);
				dr.setHeight (dirtyRects->rectangles[i].height);
				frame->platformDrawRect (&drawContext, dr);
			}
			drawContext.endDraw ();
			cairo_rectangle_list_destroy (dirtyRects);
		}
		inDraw = false;
		return true;
	}

	CButtonState buttonState (guint b, guint modifiers,
	                          GdkEventType eventType = GDK_BUTTON_PRESS) const
	{
		using namespace Gdk;
		CButtonState buttonState {};
		if (modifiers & GDK_SHIFT_MASK)
			buttonState |= kShift;
		if (modifiers & GDK_CONTROL_MASK)
			buttonState |= kControl;
		if (modifiers & GDK_MOD1_MASK)
			buttonState |= kAlt;
		if (modifiers & GDK_BUTTON1_MASK)
			buttonState |= kLButton;
		if (modifiers & GDK_BUTTON2_MASK)
			buttonState |= kMButton;
		if (modifiers & GDK_BUTTON3_MASK)
			buttonState |= kRButton;
		if (modifiers & GDK_BUTTON4_MASK)
			buttonState |= kButton4;
		if (modifiers & GDK_BUTTON5_MASK)
			buttonState |= kButton5;
		switch (b)
		{
			case 1: buttonState |= kLButton; break;
			case 2: buttonState |= kMButton; break;
			case 3: buttonState |= kRButton; break;
			case 4: buttonState |= kButton4; break;
			case 5: buttonState |= kButton5; break;
		}
		if (eventType == GDK_DOUBLE_BUTTON_PRESS)
			buttonState |= kDoubleClick;
		return buttonState;
	}

	bool on_button_press_event (GdkEventButton* event) override
	{
		if (!has_focus ())
			grab_focus ();
		CPoint where {event->x, event->y};
		auto eventButtonState = buttonState (event->button, event->state, event->type);
		frame->platformOnMouseDown (where, eventButtonState);
		return true;
	}

	bool on_button_release_event (GdkEventButton* event) override
	{
		CPoint where {event->x, event->y};
		auto eventButtonState = buttonState (event->button, event->state, event->type);
		frame->platformOnMouseUp (where, eventButtonState);
		return true;
	}

	bool on_motion_notify_event (GdkEventMotion* event) override
	{
		CPoint where {event->x, event->y};
		auto eventButtonState = buttonState (0, event->state);
		frame->platformOnMouseMoved (where, eventButtonState);
		return true;
	}

	bool on_scroll_event (GdkEventScroll* event) override
	{
		auto eventButtonState = buttonState (0, event->state);
		switch (event->direction)
		{
			case GDK_SCROLL_UP:
			{
				frame->platformOnMouseWheel ({event->x, event->y}, kMouseWheelAxisY, 1.f,
				                             eventButtonState);
				break;
			}
			case GDK_SCROLL_DOWN:
			{
				frame->platformOnMouseWheel ({event->x, event->y}, kMouseWheelAxisY, -1.f,
				                             eventButtonState);
				break;
			}
			case GDK_SCROLL_LEFT:
			{
				frame->platformOnMouseWheel ({event->x, event->y}, kMouseWheelAxisX, 1.f,
				                             eventButtonState);
				break;
			}
			case GDK_SCROLL_RIGHT:
			{
				frame->platformOnMouseWheel ({event->x, event->y}, kMouseWheelAxisX, -1.f,
				                             eventButtonState);
				break;
			}
			case GDK_SCROLL_SMOOTH:
			{
				if (event->delta_x)
					frame->platformOnMouseWheel ({event->x, event->y}, kMouseWheelAxisX,
					                             -event->delta_x, eventButtonState);
				if (event->delta_y)
					frame->platformOnMouseWheel ({event->x, event->y}, kMouseWheelAxisY,
					                             -event->delta_y, eventButtonState);
				break;
			}
		}
		return true;
	}

	bool on_key_press_event (GdkEventKey* event) override
	{
		auto keyCode = keyCodeFromEvent (event);
		return frame->platformOnKeyDown (keyCode);
	}

	bool on_key_release_event (GdkEventKey* event) override
	{
		auto keyCode = keyCodeFromEvent (event);
		return frame->platformOnKeyUp (keyCode);
	}

	void invalidRect (const CRect& rect)
	{
		if (inDraw)
			return;
		queue_draw_area (rect.left, rect.top, rect.getWidth (), rect.getHeight ());
	}

	void get_preferred_width_vfunc (int& minimumWidth, int& naturalWidth) const override
	{
		minimumWidth = 50;
		naturalWidth = size.getWidth ();
	}

	void get_preferred_height_vfunc (int& minimumHeight, int& naturalHeight) const override
	{
		minimumHeight = 50;
		naturalHeight = size.getHeight ();
	}

	bool on_drag_motion (const Glib::RefPtr<Gdk::DragContext>& context, int x, int y,
	                     guint time) override
	{
		CPoint where (x, y);
		if (!dragSourcePackage)
		{
			dragSourcePackage = owned (new GtkDragSourcePackage (*this, context));
			frame->platformOnDragEnter (dragSourcePackage, where);
		}
		else
		{
			frame->platformOnDragMove (dragSourcePackage, where);
		}
		return true;
	}

	void on_drag_leave (const Glib::RefPtr<Gdk::DragContext>& context, guint time) override
	{
		assert (dragSourcePackage);
		Glib::signal_timeout ().connect (
		    [&] () {
			    frame->platformOnDragLeave (dragSourcePackage, CPoint (-1., -1.));
				dragSourcePackage = nullptr;
				return false;
			},
		    10);
	}

	bool on_drag_drop (const Glib::RefPtr<Gdk::DragContext>& context, int x, int y,
	                   guint time) override
	{
		assert (dragSourcePackage);
		CPoint where (x, y);
		auto result = frame->platformOnDrop (dragSourcePackage, where);
		if (result)
			context->drag_finish (true, false, time);
		else
			context->drag_refuse (time);
		return true;
	}

	void on_drag_data_get (const Glib::RefPtr<Gdk::DragContext>& context,
	                       Gtk::SelectionData& selection_data, guint info, guint time) override
	{
		if (!dragPackage)
			return;
		if (dragPackage->getCount () <= info)
			return;
		IDataPackage::Type type;
		const void* buffer = nullptr;
		auto data = dragPackage->getData (info, buffer, type);
		switch (type)
		{
			case IDataPackage::kText:
			{
				selection_data.set_text (reinterpret_cast<const char*> (buffer));
				break;
			}
#warning TODO: IDataPackage::kFilePath support
#warning TODO: IDataPackage::kBinary support
			default:
			{
				assert (false && "NOT YET IMPLEMENTED");
				break;
			}
		}
	}

	void on_drag_end (const Glib::RefPtr<Gdk::DragContext>& context) override
	{
		dragPackage = nullptr;
		dragEventLoop.stop ();
	}

	DragResult doDrag (IDataPackage* source, const CPoint& offset, CBitmap* dragBitmap)
	{
#warning TODO: dragBitmap support
		auto button = 0;
		auto currentEvent = gtk_get_current_event ();
		switch (currentEvent->type)
		{
			case GDK_MOTION_NOTIFY:
			{
				if (currentEvent->motion.state & GDK_BUTTON1_MASK)
					button = 1;
				else if (currentEvent->motion.state & GDK_BUTTON2_MASK)
					button = 2;
				else if (currentEvent->motion.state & GDK_BUTTON3_MASK)
					button = 3;
				else if (currentEvent->motion.state & GDK_BUTTON4_MASK)
					button = 4;
				else if (currentEvent->motion.state & GDK_BUTTON5_MASK)
					button = 5;
				break;
			}
			case GDK_BUTTON_PRESS:
			case GDK_2BUTTON_PRESS:
			case GDK_3BUTTON_PRESS:
			{
				button = currentEvent->button.button;
				break;
			}
			default:
			{
				// it's not allowed to start a drag with anything else than a move or mouse down
				// event
				return kDragError;
			}
		}

		dragPackage = source;
		std::vector<Gtk::TargetEntry> targetEntries;
		auto itemCount = source->getCount ();
		for (auto i = 0u; i < itemCount; ++i)
		{
			auto type = source->getDataType (i);
			switch (type)
			{
				case IDataPackage::kText:
				{
					targetEntries.emplace_back ("text/plain", Gtk::TargetFlags {}, i);
					break;
				}
			}
		}
		auto targetList = Gtk::TargetList::create (targetEntries);
		Gdk::DragAction action = Gdk::ACTION_COPY | Gdk::ACTION_MOVE;
		auto dragContext =
		    drag_begin (targetList, action, button, currentEvent, offset.x, offset.y);
		dragEventLoop.run ();
		auto selectedAction = dragContext->get_selected_action ();
		if (selectedAction & Gdk::ACTION_COPY)
			return kDragCopied;
		if (selectedAction & Gdk::ACTION_MOVE)
			return kDragMoved;
		if (selectedAction == 0)
			return kDragRefused;
		return kDragError;
	}

	bool inDraw {false};
	CRect size;
	SharedPointer<IDataPackage> dragPackage;
	SharedPointer<GtkDragSourcePackage> dragSourcePackage;
	LocalEventLoop dragEventLoop;
};

//------------------------------------------------------------------------
struct Frame::Impl
{
	Impl (::Window parent) : plug (parent)
	{
		contentView.put (widget, 0, 0);
		plug.add (contentView);
	}
	Gtk::Plug plug;
	Gtk::Fixed contentView;
	GtkFrame widget;
};

//------------------------------------------------------------------------
Frame::Frame (IPlatformFrameCallback* frame, const CRect& size, uint32_t parent)
: IPlatformFrame (frame)
{
	gtk_init (0, 0);
	Gtk::Main::init_gtkmm_internals ();

	impl = std::unique_ptr<Impl> (new Impl (parent));

	impl->widget.size = size;
	impl->widget.frame = frame;
	impl->plug.show_all ();
	impl->plug.resize (size.getWidth (), size.getHeight ());

	impl->plug.property_has_toplevel_focus ().signal_changed ().connect (
	    [this] () { this->frame->platformOnActivate (impl->plug.has_toplevel_focus ()); });

#if DEBUG
	auto id = impl->plug.get_id ();
	std::cout << "PlugID: " << std::hex << id << std::endl;

//	Gdk::Event::set_show_events (true);
#endif
}

//------------------------------------------------------------------------
Frame::~Frame ()
{
}

//------------------------------------------------------------------------
bool Frame::getGlobalPosition (CPoint& pos) const
{
	int x, y;
	impl->plug.get_window ()->get_origin (x, y);
	pos = CPoint (x, y);
	return true;
}

//------------------------------------------------------------------------
bool Frame::setSize (const CRect& newSize)
{
	impl->widget.size = newSize;
	impl->plug.resize (newSize.getWidth (), newSize.getHeight ());
	return true;
}

//------------------------------------------------------------------------
bool Frame::getSize (CRect& size) const
{
	size = impl->widget.size;
	return true;
}

//------------------------------------------------------------------------
bool Frame::getCurrentMousePosition (CPoint& mousePosition) const
{
	auto dp = getDevicePosition (impl->plug);
	mousePosition = dp.second;
	return true;
}

//------------------------------------------------------------------------
bool Frame::getCurrentMouseButtons (CButtonState& buttons) const
{
	auto dp = getDevicePosition (impl->plug);
	buttons = dp.first;
	return true;
}

//------------------------------------------------------------------------
bool Frame::setMouseCursor (CCursorType type)
{
	if (auto window = impl->plug.get_window ())
	{
		const char* cursorName = nullptr;
		switch (type)
		{
			case kCursorWait: cursorName = "wait"; break;
			case kCursorHSize: cursorName = "ew-resize"; break;
			case kCursorVSize: cursorName = "ns-resize"; break;
			case kCursorSizeAll: cursorName = "crosshair"; break;
			case kCursorNESWSize: cursorName = "nesw-resize"; break;
			case kCursorNWSESize: cursorName = "nwse-resize"; break;
			case kCursorCopy: cursorName = "copy"; break;
			case kCursorNotAllowed: cursorName = "not-allowed"; break;
			case kCursorHand: cursorName = "grab"; break;
			default: cursorName = "default"; break;
		}
		if (cursorName)
		{
			auto cursor = Gdk::Cursor::create (window->get_display (), cursorName);
			window->set_cursor (cursor);
		}
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
bool Frame::invalidRect (const CRect& rect)
{
	impl->widget.invalidRect (rect);
	return true;
}

//------------------------------------------------------------------------
bool Frame::scrollRect (const CRect& src, const CPoint& distance)
{
	// optional
	(void)src;
	(void)distance;
	return false;
}

//------------------------------------------------------------------------
bool Frame::showTooltip (const CRect& rect, const char* utf8Text)
{
#warning TODO: Implementation
	return false;
}

//------------------------------------------------------------------------
bool Frame::hideTooltip ()
{
#warning TODO: Implementation
	return false;
}

//------------------------------------------------------------------------
void* Frame::getPlatformRepresentation () const
{
	return nullptr;
}

//------------------------------------------------------------------------
void* Frame::getGtkWindow ()
{
	return &impl->plug;
}

//------------------------------------------------------------------------
IPlatformTextEdit* Frame::createPlatformTextEdit (IPlatformTextEditCallback* textEdit)
{
	if (auto te = GTKTextEdit::make (&impl->contentView, textEdit))
	{
		te->remember ();
		return te;
	}
	return nullptr;
}

//------------------------------------------------------------------------
IPlatformOptionMenu* Frame::createPlatformOptionMenu ()
{
	if (auto om = new GTKOptionMenu (&impl->contentView))
		return om;
	return nullptr;
}

#if VSTGUI_OPENGL_SUPPORT
//------------------------------------------------------------------------
IPlatformOpenGLView* Frame::createPlatformOpenGLView ()
{
#warning TODO: Implementation
	return nullptr;
}
#endif

//------------------------------------------------------------------------
IPlatformViewLayer* Frame::createPlatformViewLayer (IPlatformViewLayerDelegate* drawDelegate,
                                                    IPlatformViewLayer* parentLayer)
{
	// optional
	return nullptr;
}

//------------------------------------------------------------------------
COffscreenContext* Frame::createOffscreenContext (CCoord width, CCoord height, double scaleFactor)
{
	CPoint size (width * scaleFactor, height * scaleFactor);
	auto bitmap = new Cairo::Bitmap (&size);
	bitmap->setScaleFactor (scaleFactor);
	auto context = new Cairo::Context (bitmap);
	bitmap->forget ();
	if (context->valid ())
		return context;
	context->forget ();
	return nullptr;
}

//------------------------------------------------------------------------
DragResult Frame::doDrag (IDataPackage* source, const CPoint& offset, CBitmap* dragBitmap)
{
	return impl->widget.doDrag (source, offset, dragBitmap);
}

//------------------------------------------------------------------------
void Frame::setClipboard (IDataPackage* data)
{
#warning TODO: Implementation
}

//------------------------------------------------------------------------
IDataPackage* Frame::getClipboard ()
{
#warning TODO: Implementation
	return nullptr;
}

//------------------------------------------------------------------------
void Frame::handleNextEvents ()
{
	Timer::checkAndFireTimers ();
	while (gtk_events_pending ())
	{
		gtk_main_iteration_do (false);
	}
}

//------------------------------------------------------------------------
} // X11

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
IPlatformFrame* IPlatformFrame::createPlatformFrame (IPlatformFrameCallback* frame,
                                                     const CRect& size, void* parent,
                                                     PlatformType parentType)
{
	if (parentType == kDefaultNative || parentType == kX11EmbedWindowID)
	{
		auto x11Parent = reinterpret_cast<XID> (parent);
		return new X11::Frame (frame, size, x11Parent);
	}
	return nullptr;
}

//------------------------------------------------------------------------
uint32_t IPlatformFrame::getTicks ()
{
	return static_cast<uint32_t> (X11::Platform::getCurrentTimeMs ());
}

//------------------------------------------------------------------------
} // VSTGUI
