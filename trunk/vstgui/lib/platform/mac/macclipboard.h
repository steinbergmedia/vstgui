#ifndef __macclipboard__
#define __macclipboard__

#ifdef __OBJC__
@class NSPasteboard;
#else
struct NSPasteboard;
#endif

#if MAC_CARBON
typedef struct OpaqueDragRef*           DragRef;
#endif

namespace VSTGUI {
class IDataPackage;

namespace MacClipboard {

extern IDataPackage* createClipboardDataPackage ();
extern IDataPackage* createDragDataPackage (NSPasteboard* pasteboard);
extern void setClipboard (IDataPackage* data);
extern const char* getPasteboardBinaryType ();

#if MAC_CARBON
extern IDataPackage* createCarbonDragDataPackage (DragRef drag);
#endif

}} // namespaces

#endif // __macclipboard__
