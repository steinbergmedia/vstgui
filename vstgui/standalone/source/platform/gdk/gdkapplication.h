// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include <gtkmm.h>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Platform {
namespace GDK {

Glib::RefPtr<Gtk::Application> gtkApp ();

//------------------------------------------------------------------------
} // GDK
} // Platform
} // Standalone
} // VSTGUI
