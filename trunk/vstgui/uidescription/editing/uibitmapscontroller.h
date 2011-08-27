#ifndef __uibitmapscontroller__
#define __uibitmapscontroller__

#include "../uidescription.h"

#if VSTGUI_LIVE_EDITING

#include "uiselection.h"
#include "uiundomanager.h"
#include "iaction.h"
#include "../../lib/cdatabrowser.h"

namespace VSTGUI {
class UIBitmapsDataSource;

//----------------------------------------------------------------------------------------------------
class UIBitmapsController : public CBaseObject, public DelegationController, public IGenericStringListDataBrowserSourceSelectionChanged
{
public:
	UIBitmapsController (IController* baseController, UIDescription* description, IActionPerformer* actionPerformer);
	~UIBitmapsController ();

protected:
	CView* createView (const UIAttributes& attributes, IUIDescription* description);
	CView* verifyView (CView* view, const UIAttributes& attributes, IUIDescription* description);
	CControlListener* getControlListener (UTF8StringPtr name);
	void valueChanged (CControl* pControl);

	void dbSelectionChanged (int32_t selectedRow, GenericStringListDataBrowserSource* source);

	static bool valueToString (float value, char utf8String[256], void* userData);
	static bool stringToValue (UTF8StringPtr txt, float& result, void* userData);

	SharedPointer<UIDescription> editDescription;
	IActionPerformer* actionPerformer;
	UIBitmapsDataSource* dataSource;
	SharedPointer<CView> bitmapView;
	SharedPointer<CTextEdit> bitmapPathEdit;
	SharedPointer<CControl> ninePartTiled;
	SharedPointer<CTextEdit> ninePartRectEdit[4];
	
	enum {
		kAddTag = 0,
		kRemoveTag,
		kSearchTag,
		kBitmapPathTag,
		kNinePartTiledTag,
		kNinePartTiledLeftTag,
		kNinePartTiledTopTag,
		kNinePartTiledRightTag,
		kNinePartTiledBottomTag,
	};
};

} // namespace

#endif // VSTGUI_LIVE_EDITING

#endif // __uibitmapscontroller__
