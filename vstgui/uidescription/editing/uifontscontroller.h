#ifndef __uifontscontroller__
#define __uifontscontroller__

#include "../uidescription.h"

#if VSTGUI_LIVE_EDITING

#include "uiselection.h"
#include "uiundomanager.h"
#include "iaction.h"
#include "../../lib/cdatabrowser.h"

namespace VSTGUI {
class UIFontsDataSource;
class COptionMenu;
//----------------------------------------------------------------------------------------------------
class UIFontsController : public CBaseObject, public DelegationController, public IGenericStringListDataBrowserSourceSelectionChanged
{
public:
	UIFontsController (IController* baseController, UIDescription* description, IActionPerformer* actionPerformer);
	~UIFontsController ();

protected:
	CView* createView (const UIAttributes& attributes, IUIDescription* description) VSTGUI_OVERRIDE_VMETHOD;
	CView* verifyView (CView* view, const UIAttributes& attributes, IUIDescription* description) VSTGUI_OVERRIDE_VMETHOD;
	CControlListener* getControlListener (UTF8StringPtr name) VSTGUI_OVERRIDE_VMETHOD;
	void valueChanged (CControl* pControl) VSTGUI_OVERRIDE_VMETHOD;

	void dbSelectionChanged (int32_t selectedRow, GenericStringListDataBrowserSource* source) VSTGUI_OVERRIDE_VMETHOD;

	static bool valueToString (float value, char utf8String[256], void* userData);
	static bool stringToValue (UTF8StringPtr txt, float& result, void* userData);

	SharedPointer<UIDescription> editDescription;
	IActionPerformer* actionPerformer;
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
