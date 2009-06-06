#ifndef __viewcreator__
#define __viewcreator__

#include "../vstgui.h"
#include <string>

BEGIN_NAMESPACE_VSTGUI
class IUIDescription;

extern bool parseSize (const std::string& str, CPoint& point);
extern bool pointToString (const CPoint& p, std::string& string);
extern bool bitmapToString (CBitmap* bitmap, std::string& string, IUIDescription* desc);
extern bool colorToString (const CColor& color, std::string& string, IUIDescription* desc);

extern void rememberAttributeValueString (CView* view, const char* attrName, const std::string& value);
extern bool getRememberedAttributeValueString (CView* view, const char* attrName, std::string& value);

END_NAMESPACE_VSTGUI

#endif
