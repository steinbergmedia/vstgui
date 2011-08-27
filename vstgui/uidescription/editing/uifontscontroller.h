#ifndef __uifontscontroller__
#define __uifontscontroller__

#include "../uidescription.h"

#if VSTGUI_LIVE_EDITING

#include "uiselection.h"
#include "uiundomanager.h"
#include "iactionoperation.h"
#include "../../lib/cdatabrowser.h"

namespace VSTGUI {
class UIFontsDataSource;
class COptionMenu;
//----------------------------------------------------------------------------------------------------
class UIFontsController : public CBaseObject, public DelegationController, public IGenericStringListDataBrowserSourceSelectionChanged
{
public:
	UIFontsController (IController* baseController, UIDescription* description, IActionOperator* actionOperator);
	~UIFontsController ();

protected:
	CView* createView (const UIAttributes& attributes, IUIDescription* description);
	CView* verifyView (CView* view, const UIAttributes& attributes, IUIDescription* description);
	CControlListener* getControlListener (UTF8StringPtr name);
	void valueChanged (CControl* pControl);

	void dbSelectionChanged (int32_t selectedRow, GenericStringListDataBrowserSource* source);

	static bool valueToString (float value, char utf8String[256], void* userData);
	static bool stringToValue (UTF8StringPtr txt, float& result, void* userData);

	SharedPointer<UIDescription> editDescription;
	IActionOperator* actionOperator;
	UIFontsDataSource* dataSource;

	COptionMenu* fontMenu;
	CTextEdit* altTextEdit;
	CTextEdit* sizeTextEdit;
	CControl* boldControl;
	CControl* italicControl;
	CControl* strikethroughControl;
	CControl* underlineControl;
	
	std::string selectedFont;

	enum {
		kAddTag = 0,
		kRemoveTag,
		kSearchTag,
		kFontMainTag,
		kFontAltTag,
		kFontSizeTag,
		kFontStyleBoldTag,
		kFontStyleItalicTag,
		kFontStyleStrikethroughTag,
		kFontStyleUnderlineTag,
	};
};

} // namespace

#endif // VSTGUI_LIVE_EDITING

#endif // __uifontscontroller__
