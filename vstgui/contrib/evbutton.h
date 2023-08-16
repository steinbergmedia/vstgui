// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../lib/iexternalview.h"
#include "../lib/cstring.h"
#include <memory>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace ExternalView {

//------------------------------------------------------------------------
class Button : public ControlViewAdapter
{
public:
	enum class Type
	{
		Checkbox,
		Push,
		Radio,
		OnOff
	};

	Button (Type type, const UTF8String& title);
	~Button () noexcept;

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

	bool setValue (double value) override;
	bool setEditCallbacks (const EditCallbacks& callbacks) override;

	struct Impl;
	std::unique_ptr<Impl> impl;
};

//------------------------------------------------------------------------
} // ExternalView
} // VSTGUI
