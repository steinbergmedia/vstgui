#include <gtkmm.h> // must be first!

#include "../../vstkeycode.h"
#include "gtktextedit.h"
#include <functional>
#include <sstream>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace X11 {
extern VstKeyCode keyCodeFromEvent (GdkEventKey* event);
} // X11
//------------------------------------------------------------------------
struct GTKTextField : Gtk::Entry
{
	using KeyPressFunc = std::function<bool (VstKeyCode)>;
	using LostFocusFunc = std::function<void ()>;

	GTKTextField () {}

	bool on_key_press_event (GdkEventKey* event) override
	{
		if (keyPress)
		{
			if (keyPress (X11::keyCodeFromEvent (event)))
				return false;
		}
		return Gtk::Entry::on_key_press_event (event);
	}

	void get_preferred_width_vfunc (int& minimumWidth, int& naturalWidth) const override
	{
		minimumWidth = naturalWidth = width;
	}

	bool on_focus_out_event (GdkEventFocus* event) override
	{
		auto result = Gtk::Entry::on_focus_out_event (event);
		if (lostFocus)
			lostFocus ();
		return result;
	}

	KeyPressFunc keyPress;
	LostFocusFunc lostFocus;
	int width {100};
};

//------------------------------------------------------------------------
struct GTKTextEdit::Impl
{
	GTKTextField widget;
	Gtk::Fixed* parent;
	std::string text;
};

//------------------------------------------------------------------------
GTKTextEdit::GTKTextEdit (std::unique_ptr<Impl>&& inImpl, IPlatformTextEditCallback* callback)
: IPlatformTextEdit (callback), impl (std::move (inImpl))
{
	auto size = callback->platformGetSize ();
	impl->widget.width = static_cast<int> (size.getWidth ());
	impl->widget.set_size_request (size.getWidth (), size.getHeight ());
	impl->widget.set_can_focus (true);
	if (auto text = callback->platformGetText ())
	{
		impl->widget.set_text (text);
		impl->widget.select_region (0, impl->widget.get_text_length ());
	}
	impl->parent->put (impl->widget, size.left, size.top);
	impl->parent->show_all ();
	impl->widget.grab_focus ();
	impl->widget.keyPress = [callback] (VstKeyCode keyCode) {
		return callback->platformOnKeyDown (keyCode);
	};
	impl->widget.lostFocus = [callback] () { callback->platformLooseFocus (false); };

	impl->widget.get_buffer ()->signal_inserted_text ().connect (
		[callback] (guint, const gchar*, guint) { callback->platformTextDidChange (); }, true);
	impl->widget.get_buffer ()->signal_deleted_text ().connect (
		[callback] (guint, guint) { callback->platformTextDidChange (); }, true);

	Gtk::Align alignment = Gtk::ALIGN_CENTER;
	switch (callback->platformGetHoriTxtAlign ())
	{
		case kLeftText: alignment = Gtk::ALIGN_START; break;
		case kRightText: alignment = Gtk::ALIGN_END; break;
	}

	impl->widget.set_alignment (alignment);

	// TODO: finalize visual adaption via GTK CSS stuff
	auto font = callback->platformGetFont ();
	auto color = callback->platformGetFontColor ();
	if (auto style = impl->widget.get_style_context ())
	{
		std::stringstream stream;
		stream << "* { border-radius: 0; border-style: none; padding: 0;";
		stream << "font: " << font->getName () << " " << font->getSize () << "; ";
		stream << "color: rgba(" << static_cast<uint32_t> (color.red) << ", "
			   << static_cast<uint32_t> (color.green) << ", " << static_cast<uint32_t> (color.blue)
			   << ", " << (color.alpha / 255.) << ");";
		//		stream << "background: rgba(0, 0, 0, 0); "; // on linux-mint this is also the
		//selection-color
		stream << "background-image: linear-gradient(rgba(0, 0, 0, 0)); ";
		stream << "}";
		auto provider = Gtk::CssProvider::create ();
		auto streamStr = stream.str ();
		provider->load_from_data (streamStr);
		style->add_provider (provider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
	}
}

//------------------------------------------------------------------------
GTKTextEdit::~GTKTextEdit ()
{
}

//------------------------------------------------------------------------
UTF8StringPtr GTKTextEdit::getText ()
{
	impl->text = impl->widget.get_text ().data ();
	return impl->text.data ();
}

//------------------------------------------------------------------------
bool GTKTextEdit::setText (UTF8StringPtr text)
{
	impl->widget.set_text (text);
	return true;
}

//------------------------------------------------------------------------
bool GTKTextEdit::updateSize ()
{
	return false;
}

//------------------------------------------------------------------------
SharedPointer<GTKTextEdit> GTKTextEdit::make (void* parentWidget,
											  IPlatformTextEditCallback* callback)
{
	auto impl = std::unique_ptr<GTKTextEdit::Impl> (new Impl ());
	impl->parent = reinterpret_cast<Gtk::Fixed*> (parentWidget);
	return owned (new GTKTextEdit (std::move (impl), callback));
}

//------------------------------------------------------------------------
} // VSTGUI
