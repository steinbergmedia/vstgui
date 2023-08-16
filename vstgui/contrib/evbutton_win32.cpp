// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "evbutton.h"
#include "externalview_hwnd.h"
#include "vstgui/lib/platform/win32/win32factory.h"
#include "vstgui/lib/platform/win32/winstring.h"

#include <windowsx.h>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace ExternalView {

//------------------------------------------------------------------------
struct Button::Impl : ExternalHWNDBase,
					  IControlViewExtension
{
	using ExternalHWNDBase::ExternalHWNDBase;

	Type type {};
	EditCallbacks callbacks {};
	double value {0.};

	bool setValue (double val) override
	{
		value = val;
		auto state = Button_GetState (child);
		switch (type)
		{
			case Type::Checkbox:
			case Type::Radio:
			{
				Button_SetCheck (child, value > 0.5);
				break;
			}
				{
					break;
				}
			case Type::OnOff:
			{
				if (value == 0)
					state = 0; //~BST_PUSHED;
				else
					state = BST_PUSHED;
				Button_SetState (child, state);
				break;
			}
			case Type::Push:
			{
				break;
			}
		}
		return true;
	}

	bool setEditCallbacks (const EditCallbacks& editCallbacks) override
	{
		callbacks = editCallbacks;
		return true;
	}

	void onButtonClick (double val)
	{
		if (callbacks.beginEdit)
			callbacks.beginEdit ();
		if (callbacks.performEdit)
			callbacks.performEdit (val);
		if (callbacks.endEdit)
			callbacks.endEdit ();
	}
};

//------------------------------------------------------------------------
Button::Button (Type type, const UTF8String& inTitle)
{
	DWORD addStyle = 0;
	switch (type)
	{
		case Type::Checkbox:
			addStyle = BS_AUTOCHECKBOX;
			break;
		case Type::Radio:
			addStyle = BS_RADIOBUTTON;
			break;
		case Type::OnOff:
			addStyle = BS_PUSHBUTTON;
			break;
		case Type::Push:
			addStyle = BS_PUSHBUTTON;
			break;
	}
	auto winString = dynamic_cast<WinString*> (inTitle.getPlatformString ());
	auto hInstance = getPlatformFactory ().asWin32Factory ()->getInstance ();
	impl = std::make_unique<Impl> (hInstance);
	impl->type = type;
	impl->child = CreateWindowExW (WS_EX_COMPOSITED, TEXT ("BUTTON"),
								   winString ? winString->getWideString () : nullptr,
								   WS_CHILD | WS_VISIBLE | BS_TEXT | addStyle, 0, 0, 80, 20,
								   impl->container.getHWND (), NULL, hInstance, NULL);
	impl->container.setWindowProc (
		[this] (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) -> LRESULT {
			switch (message)
			{
				case WM_COMMAND:
				{
					if (HIWORD (wParam) == BN_CLICKED)
					{
						switch (impl->type)
						{
							case Type::Checkbox:
							{
								impl->onButtonClick (impl->value == 0. ? 1 : 0.);
								break;
							}
							case Type::Radio:
							{
								impl->onButtonClick (impl->value == 0. ? 1 : 0.);
								break;
							}
							case Type::OnOff:
							{
								impl->onButtonClick (impl->value == 0. ? 1 : 0.);
								break;
							}
							case Type::Push:
							{
								impl->onButtonClick (1.);
								impl->onButtonClick (0.);
								break;
							}
						}
						return 0;
					}
					break;
				}
				case WM_ERASEBKGND:
					return 0;
			}
			return DefWindowProc (hwnd, message, wParam, lParam);
		});
}

//------------------------------------------------------------------------
Button::~Button () noexcept = default;

//------------------------------------------------------------------------
bool Button::platformViewTypeSupported (PlatformViewType type)
{
	return impl->platformViewTypeSupported (type);
}

//------------------------------------------------------------------------
bool Button::attach (void* parent, PlatformViewType parentViewType)
{
	return impl->attach (parent, parentViewType);
}

//------------------------------------------------------------------------
bool Button::remove () { return impl->remove (); }

//------------------------------------------------------------------------
void Button::setViewSize (IntRect frame, IntRect visible) { impl->setViewSize (frame, visible); }

//------------------------------------------------------------------------
void Button::setContentScaleFactor (double scaleFactor)
{
	impl->setContentScaleFactor (scaleFactor);
}

//------------------------------------------------------------------------
void Button::setMouseEnabled (bool state) { impl->setMouseEnabled (state); }

//------------------------------------------------------------------------
void Button::takeFocus () { impl->takeFocus (); }

//------------------------------------------------------------------------
void Button::looseFocus () { impl->looseFocus (); }

//------------------------------------------------------------------------
void Button::setTookFocusCallback (const TookFocusCallback& callback)
{
	impl->setTookFocusCallback (callback);
}

//------------------------------------------------------------------------
bool Button::setValue (double value) { return impl->setValue (value); }

//------------------------------------------------------------------------
bool Button::setEditCallbacks (const EditCallbacks& callbacks)
{
	return impl->setEditCallbacks (callbacks);
}

//------------------------------------------------------------------------
} // ExternalView
} // VSTGUI
