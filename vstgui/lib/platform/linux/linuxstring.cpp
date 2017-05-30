// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "linuxstring.h"

//------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------
SharedPointer<IPlatformString> IPlatformString::createWithUTF8String (UTF8StringPtr utf8String)
{
	return owned (new LinuxString (utf8String));
}

//------------------------------------------------------------------------
LinuxString::LinuxString (UTF8StringPtr utf8String) : str (utf8String)
{
}

//------------------------------------------------------------------------
void LinuxString::setUTF8String (UTF8StringPtr utf8String)
{
	str = utf8String ? utf8String : "";
}

//------------------------------------------------------------------------
}
