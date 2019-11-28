// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

/// @cond ignore

#include "../lib/vstguifwd.h"
#include <string>

//------------------------------------------------------------------------
namespace VSTGUI {
class IUIDescription;

//------------------------------------------------------------------------
namespace UIViewCreator {

VSTGUI_DEPRECATED (extern bool parseSize (const std::string& str, CPoint& point);)
VSTGUI_DEPRECATED (extern bool pointToString (const CPoint& p, std::string& string);)
extern bool bitmapToString (CBitmap* bitmap, std::string& string, const IUIDescription* desc);
extern bool colorToString (const CColor& color, std::string& string, const IUIDescription* desc);
extern bool stringToColor (const std::string* value, CColor& color, const IUIDescription* desc);
extern bool stringToBitmap (const std::string* value, CBitmap*& bitmap, const IUIDescription* desc);
extern void applyStyleMask (const std::string* value, int32_t mask, int32_t& style);

extern bool getStandardAttributeListValues (const std::string& attributeName,
                                            std::list<const std::string*>& values);
extern void addGradientToUIDescription (const IUIDescription* description, CGradient* gradient,
                                        UTF8StringPtr baseName);

//------------------------------------------------------------------------
} // UIViewCreator
} // VSTGUI

/// @endcond
