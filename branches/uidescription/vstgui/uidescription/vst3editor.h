
#ifndef __vst3editor__
#define __vst3editor__

#include "public.sdk/source/vst/vstguieditor.h"
#include "uidescription.h"
#include "../ctooltipsupport.h"
#include <string>
#include <map>

#ifdef verify
#undef verify
#endif

BEGIN_NAMESPACE_VSTGUI

class ParameterChangeListener;

//-----------------------------------------------------------------------------
#define kVST3EditorReloadDescriptionTagName	"VST3Editor::reloadDescriptionTag"

//-----------------------------------------------------------------------------
class VST3Editor : public Steinberg::Vst::VSTGUIEditor, public IController
{
public:
	VST3Editor (void* controller, const char* viewName, const char* xmlFile, bool debugMode = false);
	VST3Editor (UIDescription* desc, void* controller, const char* viewName, const char* xmlFile = 0, bool debugMode = false);

	bool verify ();
	bool exchangeView (const char* newViewName);
	void enableTooltips (bool state);

protected:
	~VST3Editor ();
	void init ();
	ParameterChangeListener* getParameterChangeListener (long tag);
	void reloadDescription ();
	void recreateView ();
	
	bool PLUGIN_API open (void* parent);
	void PLUGIN_API close ();

	void beginEdit (long index);
	void endEdit (long index);

	CView* verifyView (CView* view, const UIAttributes& attributes, UIDescription* description);
	CMessageResult notify (CBaseObject* sender, const char* message);

	Steinberg::tresult PLUGIN_API onSize (Steinberg::ViewRect* newSize);
	Steinberg::tresult PLUGIN_API canResize ();
	Steinberg::tresult PLUGIN_API checkSizeConstraint (Steinberg::ViewRect* rect);

	// CControlListener
	virtual void valueChanged (CControl* pControl);
	virtual void controlBeginEdit (CControl* pControl);
	virtual void controlEndEdit (CControl* pControl);

	UIDescription* description;
	CTooltipSupport* tooltipSupport;
	std::map<long, ParameterChangeListener*> paramChangeListeners;
	std::string viewName;
	std::string xmlFile;
	bool debugMode;
	bool tooltipsEnabled;
	bool doReloadDescription;
	bool doCreateView;
	long reloadDescriptionTag;
	
	CPoint minSize;
	CPoint maxSize;
};

END_NAMESPACE_VSTGUI

#endif
