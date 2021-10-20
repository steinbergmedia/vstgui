// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../../vstguibase.h"

#ifdef __OBJC__
@class NSPasteboard;
#else
struct NSPasteboard;
#endif

namespace VSTGUI {
class IDataPackage;

namespace MacClipboard {

extern SharedPointer<IDataPackage> createClipboardDataPackage ();
extern SharedPointer<IDataPackage> createDragDataPackage (NSPasteboard* pasteboard);
extern void setClipboard (const SharedPointer<IDataPackage>& data);
extern const char* getPasteboardBinaryType ();

}} // namespaces
