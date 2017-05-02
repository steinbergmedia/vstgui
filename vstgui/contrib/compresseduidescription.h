/*
 *  compresseduidescription.h
 *
 *
 *  Created by Arne Scheffler on 10/19/10.
 *  Copyright 2010 Arne Scheffler. All rights reserved.
 *
 */

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
