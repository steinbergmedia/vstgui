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

#include "gtkoptionmenu.h"
#include "../../cbitmap.h"
#include "../../controls/coptionmenu.h"
#include "cairobitmap.h"
#include "x11platform.h"
#include <chrono>
#include <gtkmm.h>

//------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------
struct GTKOptionMenu::Impl
{
	Gtk::Fixed* parent {nullptr};

	CPoint position;
	void on_popup_menu_position (int& x, int& y, bool& push_in)
	{
		x = position.x;
		y = position.y;
		push_in = false;
	}
};

//------------------------------------------------------------------------
GTKOptionMenu::GTKOptionMenu (void* parent)
{
	impl = std::unique_ptr<Impl> (new Impl);
	impl->parent = reinterpret_cast<Gtk::Fixed*> (parent);
}

//------------------------------------------------------------------------
GTKOptionMenu::~GTKOptionMenu ()
{
}

//------------------------------------------------------------------------
static Glib::RefPtr<Gdk::Pixbuf> pixBufFromBitmap (CBitmap* bitmap)
{
	if (!bitmap)
		return {};
	auto platformBitmap = bitmap->getPlatformBitmap ();
	if (!platformBitmap)
		return {};
	auto cairoBitmap = dynamic_cast<Cairo::Bitmap*> (platformBitmap);
	if (!cairoBitmap)
		return {};
	auto surface = cairoBitmap->getSurface ();
	if (!surface)
		return {};
	auto pixBuf = gdk_pixbuf_get_from_surface (surface, 0, 0, cairoBitmap->getSize ().x,
											   cairoBitmap->getSize ().y);
	return Glib::wrap (pixBuf);
}

//------------------------------------------------------------------------
static void constructMenu (Gtk::Menu* menu, COptionMenu* optionMenu,
						   PlatformOptionMenuResult& result)
{
	menu->set_reserve_toggle_size (false);
	auto style = optionMenu->getStyle ();
	auto checkStyle = style & kCheckStyle || style & kMultipleCheckStyle;
	auto popupStyle = style & kPopupStyle;
	int32_t index = 0;
	for (auto item : *optionMenu->getItems ())
	{
		if (item->isSeparator ())
		{
			auto menuItem = Gtk::manage (new Gtk::SeparatorMenuItem ());
			menuItem->show_all ();
			menu->append (*menuItem);
		}
		else
		{
			auto menuItem = Gtk::manage (new Gtk::MenuItem ());
			auto box = Gtk::manage (new Gtk::HBox ());
			box->set_spacing (5);
			if (checkStyle)
			{
				bool checked =
					item->isChecked () || (checkStyle && popupStyle && optionMenu->getValue () == index);
				auto checkLabel = Gtk::manage (new Gtk::Label (
					checked ? u8"\xE2\x9C\x93" : u8"\xE2\x80\x83", Gtk::ALIGN_START));
				checkLabel->set_size_request (15, 15);
				box->pack_start (*checkLabel, Gtk::PACK_SHRINK, 0);
			}
			if (auto pixBuf = pixBufFromBitmap (item->getIcon ()))
			{
				auto image = Gtk::manage (new Gtk::Image (pixBuf));
				box->pack_start (*image, false, false, 0);
			}
			auto titleLabel = Gtk::manage (new Gtk::Label (item->getTitle (), Gtk::ALIGN_START));
			if (popupStyle && optionMenu->getValue () == index)
			{
				std::stringstream stream;
				stream << "<b>" << item->getTitle () << "</b>";
				titleLabel->set_markup (stream.str ());
			}
			box->pack_start (*titleLabel, true, true, 5);
			menuItem->add (*box);
			menuItem->signal_activate ().connect ([index, optionMenu, &result] () {
				result.index = index;
				result.menu = optionMenu;
			});
			if (auto subOptionMenu = item->getSubmenu ())
			{
				auto subMenu = Gtk::manage (new Gtk::Menu);
				constructMenu (subMenu, subOptionMenu, result);
				menuItem->set_submenu (*subMenu);
			}
			if (item->isTitle ())
			{
				titleLabel->set_padding (15, 0);
				menuItem->set_sensitive (false);
			}
			else
				menuItem->set_sensitive (item->isEnabled ());

			menuItem->show_all ();
			menu->append (*menuItem);
		}
		++index;
	}
}

//------------------------------------------------------------------------
PlatformOptionMenuResult GTKOptionMenu::popup (COptionMenu* optionMenu)
{
	PlatformOptionMenuResult result {};

	auto menu = Gtk::manage (new Gtk::Menu);
	constructMenu (menu, optionMenu, result);

	auto size = optionMenu->translateToGlobal (optionMenu->getViewSize ());
	if (auto window = impl->parent->get_window ())
	{
		int x, y;
		window->get_origin (x, y);
		size.offset (x, y);
	}
	guint32 eventTime = 0;
	guint button = 0;
	if (auto currentEvent = gtk_get_current_event ())
	{
		switch (currentEvent->any.type)
		{
			case GDK_BUTTON_PRESS:
			case GDK_2BUTTON_PRESS:
			case GDK_3BUTTON_PRESS:
			case GDK_BUTTON_RELEASE:
			{
				eventTime = currentEvent->button.time;
				button = currentEvent->button.button;
				break;
			}
			default:
				break;
		}
	}
	if (eventTime == 0)
		eventTime = gtk_get_current_event_time ();

	X11::LocalEventLoop eventLoop;
	menu->signal_deactivate ().connect ([&] () { eventLoop.stop (); });

	menu->popup (
		[&] (int& x, int& y, bool& push_in) {
			x = size.left;
			if (optionMenu->getStyle () & kPopupStyle)
			{
				auto selectedIndex = static_cast<uint32_t> (optionMenu->getValue ());
				if (selectedIndex != 0)
				{
					auto children = menu->get_children ();
					if (children.size () > selectedIndex)
					{
						if (auto item = dynamic_cast<Gtk::MenuItem*> (children[selectedIndex]))
						{
							auto allocation = item->get_allocation ();
							size.top -= allocation.get_y ();
						}
					}
				}
				y = size.top;
			}
			else
			{
				y = size.bottom;
			}
		},
		button, eventTime);

	eventLoop.run ();
	return result;
}

//------------------------------------------------------------------------
}
