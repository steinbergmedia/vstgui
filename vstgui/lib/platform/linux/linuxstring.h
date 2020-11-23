// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../iplatformstring.h"
#include <string>

//------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------
class LinuxString : public IPlatformString
{
public:
	LinuxString (UTF8StringPtr utf8String);
	virtual void setUTF8String (UTF8StringPtr utf8String) override;

	const std::string& get () const { return str; }
private:
	std::string str;
};

//------------------------------------------------------------------------
}
