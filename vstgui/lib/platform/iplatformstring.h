// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __iplatformstring__
#define __iplatformstring__

#include "../vstguibase.h"

/// @cond ignore

namespace VSTGUI {

//-----------------------------------------------------------------------------
class IPlatformString : public AtomicReferenceCounted
{
public:
	static SharedPointer<IPlatformString> createWithUTF8String (UTF8StringPtr utf8String = nullptr);
	
	virtual void setUTF8String (UTF8StringPtr utf8String) = 0;
};

} // namespace

/// @endcond

#endif // __iplatformstring__
