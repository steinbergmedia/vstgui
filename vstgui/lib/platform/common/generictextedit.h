// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../iplatformtextedit.h"
#include <memory>

//-----------------------------------------------------------------------------
namespace VSTGUI {

//-----------------------------------------------------------------------------
class GenericTextEdit : public IPlatformTextEdit
{
public:
	GenericTextEdit (IPlatformTextEditCallback* callback);
    ~GenericTextEdit () noexcept;

	UTF8String getText () override;
	bool setText (const UTF8String& text) override;
	bool updateSize () override;
	bool drawsPlaceholder () const override { return false; }

private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};

//-----------------------------------------------------------------------------
} // VSTGUI
