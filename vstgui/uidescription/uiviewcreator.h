// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __uiviewcreator__
#define __uiviewcreator__

/// @cond ignore

#include "../lib/vstguifwd.h"
#include <string>

namespace VSTGUI {
class IUIDescription;

namespace UIViewCreator {

extern bool parseSize (const std::string& str, CPoint& point);
extern bool pointToString (const CPoint& p, std::string& string);
extern bool bitmapToString (CBitmap* bitmap, std::string& string, const IUIDescription* desc);
extern bool colorToString (const CColor& color, std::string& string, const IUIDescription* desc);

} } // namespaces

/// @endcond

#endif // __uiviewcreator__
