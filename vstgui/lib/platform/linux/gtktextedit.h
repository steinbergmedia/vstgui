#pragma once

#include "../iplatformtextedit.h"
#include <memory>

//------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------
class GTKTextEdit : public IPlatformTextEdit
{
public:
	static SharedPointer<GTKTextEdit> make (void* parentWidget,
											IPlatformTextEditCallback* callback);

	~GTKTextEdit ();

	UTF8String getText () override;
	bool setText (const UTF8String& text) override;
	bool updateSize () override;

private:
	struct Impl;

	GTKTextEdit (std::unique_ptr<Impl>&& impl, IPlatformTextEditCallback* callback);

	std::unique_ptr<Impl> impl;
};

//------------------------------------------------------------------------
} // VSTGUI
