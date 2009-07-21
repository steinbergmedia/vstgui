
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
class IVST3CustomViewCreator
{
public:
	virtual ~IVST3CustomViewCreator () {}
	
	virtual CView* createCustomView (const char* name, const UIAttributes& attributes, IUIDescription* description) = 0;
};

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
	void recreateView ();

	#if VSTGUI_LIVE_EDITING
	void runNewTemplateDialog (const char* baseViewName);
	void runTemplateSettingsDialog ();
	#endif // VSTGUI_LIVE_EDITING
	
	bool PLUGIN_API open (void* parent);
	void PLUGIN_API close ();

	void beginEdit (long index);
	void endEdit (long index);

	CView* createView (const UIAttributes& attributes, IUIDescription* description);
	CView* verifyView (CView* view, const UIAttributes& attributes, IUIDescription* description);
	CMessageResult notify (CBaseObject* sender, const char* message);

	Steinberg::tresult PLUGIN_API onSize (Steinberg::ViewRect* newSize);
	Steinberg::tresult PLUGIN_API canResize ();
	Steinberg::tresult PLUGIN_API checkSizeConstraint (Steinberg::ViewRect* rect);

	Steinberg::tresult PLUGIN_API onWheel (float distance);
	Steinberg::tresult PLUGIN_API onKeyDown (Steinberg::char16 key, Steinberg::int16 keyMsg, Steinberg::int16 modifiers);
	Steinberg::tresult PLUGIN_API onKeyUp (Steinberg::char16 key, Steinberg::int16 keyMsg, Steinberg::int16 modifiers);

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
	bool doCreateView;
	
	CPoint minSize;
	CPoint maxSize;
};

END_NAMESPACE_VSTGUI

#endif
