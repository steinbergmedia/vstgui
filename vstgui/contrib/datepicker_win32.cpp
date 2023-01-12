
#include "datepicker.h"
#include "externalview_hwnd.h"
#include "vstgui/lib/platform/win32/win32factory.h"

#include <CommCtrl.h>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace ExternalView {

//------------------------------------------------------------------------
struct DatePicker::Impl : ExternalHWNDBase
{
	using Base::Base;

	~Impl () noexcept
	{
		if (font)
			DeleteObject (font);
	}

	ChangeCallback changeCallback;
	HFONT font {nullptr};
};

//------------------------------------------------------------------------
DatePicker::DatePicker ()
{
	auto hInstance = getPlatformFactory ().asWin32Factory ()->getInstance ();
	impl = std::make_unique<Impl> (hInstance);
	impl->child = CreateWindowExW (0, DATETIMEPICK_CLASS, TEXT ("DateTime"),
								   WS_BORDER | WS_CHILD | WS_VISIBLE | DTS_SHORTDATEFORMAT, 0, 0,
								   80, 20, impl->container.getHWND (), NULL, hInstance, NULL);
	impl->container.setWindowProc ([this] (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
		switch (message)
		{
			case WM_NOTIFY:
			{
				LPNMHDR hdr = reinterpret_cast<LPNMHDR> (lParam);
				switch (hdr->code)
				{
					case DTN_DATETIMECHANGE:
					{
						LPNMDATETIMECHANGE lpChange = reinterpret_cast<LPNMDATETIMECHANGE> (lParam);
						if (impl->changeCallback)
						{
							Date date;
							date.day = lpChange->st.wDay;
							date.month = lpChange->st.wMonth;
							date.year = lpChange->st.wYear;
							impl->changeCallback (date);
						}
						break;
					}
				}
				break;
			}
		}
		return DefWindowProc (hwnd, message, wParam, lParam);
	});
}

//------------------------------------------------------------------------
DatePicker::~DatePicker () noexcept {}

//------------------------------------------------------------------------
void DatePicker::setDate (Date date)
{
	SYSTEMTIME st = {};
	st.wDay = date.day;
	st.wMonth = date.month;
	st.wYear = date.year;
	DateTime_SetSystemtime (impl->child, GDT_VALID, &st);
}

//------------------------------------------------------------------------
void DatePicker::setChangeCallback (const ChangeCallback& callback)
{
	impl->changeCallback = callback;
}

//------------------------------------------------------------------------
bool DatePicker::platformViewTypeSupported (PlatformViewType type)
{
	return impl->platformViewTypeSupported (type);
}

//------------------------------------------------------------------------
bool DatePicker::attach (void* parent, PlatformViewType parentViewType)
{
	return impl->attach (parent, parentViewType);
}

//------------------------------------------------------------------------
bool DatePicker::remove () { return impl->remove (); }

//------------------------------------------------------------------------
void DatePicker::setViewSize (IntRect frame, IntRect visible)
{
	impl->setViewSize (frame, visible);
}

//------------------------------------------------------------------------
void DatePicker::setContentScaleFactor (double scaleFactor)
{
	if (impl->font)
		DeleteObject (impl->font);
	auto logFont = NonClientMetrics::get ().lfCaptionFont;
	logFont.lfHeight = static_cast<LONG> (std::round (logFont.lfHeight * scaleFactor));
	impl->font = CreateFontIndirect (&logFont);
	if (impl->font)
		SendMessage (impl->child, WM_SETFONT, (WPARAM)impl->font, 0);
}

//------------------------------------------------------------------------
void DatePicker::setMouseEnabled (bool state) { impl->setMouseEnabled (state); }

//------------------------------------------------------------------------
void DatePicker::takeFocus () { impl->takeFocus (); }

//------------------------------------------------------------------------
void DatePicker::looseFocus () { impl->looseFocus (); }

void DatePicker::setTookFocusCallback (const TookFocusCallback& callback)
{
	impl->setTookFocusCallback (callback);
}

//------------------------------------------------------------------------
} // ExternalView
} // VSTGUI
