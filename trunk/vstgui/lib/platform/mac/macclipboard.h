#ifndef __macclipboard__
#define __macclipboard__

namespace VSTGUI {
class IDataPackage;

namespace MacClipboard {

extern IDataPackage* getClipboard ();
extern void setClipboard (IDataPackage* data);
extern const char* getPasteboardBinaryType ();

}} // namespaces

#endif // __macclipboard__
