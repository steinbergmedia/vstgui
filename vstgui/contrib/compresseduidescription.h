// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __compresseduidescription__
#define __compresseduidescription__

#include "vstgui/uidescription/uidescription.h"

//------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------
class CompressedUIDescription : public UIDescription
{
public:
	CompressedUIDescription (const CResourceDescription& compressedUIDescFile);

	bool parse () override;
	bool save (UTF8StringPtr filename, int32_t flags = kWriteWindowsResourceFile) override;
};

//------------------------------------------------------------------------
} // namespace

#endif
