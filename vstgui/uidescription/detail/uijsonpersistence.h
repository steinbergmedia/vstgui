// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../cstream.h"
#include "../icontentprovider.h"
#include "uinode.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Detail {

//------------------------------------------------------------------------
namespace UIJsonDescReader {

//------------------------------------------------------------------------
SharedPointer<UINode> read (IContentProvider& contentProvider);

//------------------------------------------------------------------------
} // UIJsonDescReader

//------------------------------------------------------------------------
namespace UIJsonDescWriter {

//------------------------------------------------------------------------
bool write (OutputStream& stream, UINode* rootNode, bool pretty = true);

//------------------------------------------------------------------------
} // UIJsonDescWriter

//------------------------------------------------------------------------
} // Detail
} // VSTGUI
