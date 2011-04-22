
#ifndef __iplatformstring__
#define __iplatformstring__

#include "../vstguibase.h"

namespace VSTGUI {

class IPlatformString : public CBaseObject
{
public:
	static IPlatformString* createWithUTF8String (UTF8StringPtr utf8String = 0);
	
	virtual void setUTF8String (UTF8StringPtr utf8String) = 0;
};

} // namespace

#endif // __iplatformstring__
