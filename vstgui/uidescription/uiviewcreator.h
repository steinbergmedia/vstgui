// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

/// @cond ignore

#include "../lib/vstguifwd.h"
#include <string>
#include <list>

//------------------------------------------------------------------------
namespace VSTGUI {
class IUIDescription;

//------------------------------------------------------------------------
namespace UIViewCreator {

bool bitmapToString (const SPtr<CBitmap>& bitmap, std::string& string, const IUIDescription& desc);
bool colorToString (const CColor& color, std::string& string, const IUIDescription& desc);
bool stringToColor (const std::string* value, CColor& color, const IUIDescription& desc);
bool stringToColor (std::string_view value, CColor& color, const IUIDescription& desc);
bool stringToBitmap (const std::string* value, SPtr<CBitmap>& bitmap, const IUIDescription& desc);
void applyStyleMask (const std::string* value, int32_t mask, int32_t& style);

bool getStandardAttributeListValues (const std::string& attributeName,
									 std::list<const std::string*>& values);
void addGradientToUIDescription (const IUIDescription& description, const SPtr<CGradient>& gradient,
								 UTF8StringPtr baseName);

void forceLinking ();

//------------------------------------------------------------------------
} // UIViewCreator
} // VSTGUI

/// @endcond
