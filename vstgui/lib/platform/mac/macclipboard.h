// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __macclipboard__
#define __macclipboard__

#include "../../vstguibase.h"

#ifdef __OBJC__
@class NSPasteboard;
#else
struct NSPasteboard;
#endif

#if MAC_CARBON
using DragRef = struct OpaqueDragRef*;
#endif

namespace VSTGUI {
class IDataPackage;

namespace MacClipboard {

extern SharedPointer<IDataPackage> createClipboardDataPackage ();
extern SharedPointer<IDataPackage> createDragDataPackage (NSPasteboard* pasteboard);
extern void setClipboard (const SharedPointer<IDataPackage>& data);
extern const char* getPasteboardBinaryType ();

#if MAC_CARBON
extern SharedPointer<IDataPackage> createCarbonDragDataPackage (DragRef drag);
#endif

}} // namespaces

#endif // __macclipboard__
