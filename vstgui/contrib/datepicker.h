// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../lib/iexternalview.h"
#include <functional>
#include <memory>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace ExternalView {

//------------------------------------------------------------------------
class DatePicker : public ViewAdapter
{
public:
	DatePicker ();
	~DatePicker () noexcept;

	struct Date
	{
		int32_t day {0};
		int32_t month {0};
		int32_t year {0};
	};
	void setDate (Date date);

	using ChangeCallback = std::function<void (Date)>;
	void setChangeCallback (const ChangeCallback& callback);

private:
	bool platformViewTypeSupported (PlatformViewType type) override;
	bool attach (void* parent, PlatformViewType parentViewType) override;
	bool remove () override;

	void setViewSize (IntRect frame, IntRect visible) override;
	void setContentScaleFactor (double scaleFactor) override;

	void setMouseEnabled (bool state) override;

	void takeFocus () override;
	void looseFocus () override;

	void setTookFocusCallback (const TookFocusCallback& callback) override;

	struct Impl;
	std::unique_ptr<Impl> impl;
};

//------------------------------------------------------------------------
} // ExternalView
} // VSTGUI
