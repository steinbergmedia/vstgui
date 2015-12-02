//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 4.3
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2015, Steinberg Media Technologies, All Rights Reserved
//-----------------------------------------------------------------------------
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
// 
//   * Redistributions of source code must retain the above copyright notice, 
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation 
//     and/or other materials provided with the distribution.
//   * Neither the name of the Steinberg Media Technologies nor the names of its
//     contributors may be used to endorse or promote products derived from this 
//     software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#ifndef __uiviewfactory__
#define __uiviewfactory__

#include "../lib/vstguifwd.h"
#include "iuidescription.h"
#include "iviewfactory.h"
#include "iviewcreator.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
/// @brief Default view factory
///	@ingroup new_in_4_0
//-----------------------------------------------------------------------------
class UIViewFactory : public CBaseObject, public IViewFactory
{
public:
	UIViewFactory ();
	~UIViewFactory ();

	// IViewFactory
	CView* createView (const UIAttributes& attributes, const IUIDescription* description) const VSTGUI_OVERRIDE_VMETHOD;
	bool applyAttributeValues (CView* view, const UIAttributes& attributes, const IUIDescription* desc) const VSTGUI_OVERRIDE_VMETHOD;
	IdStringPtr getViewName (CView* view) const VSTGUI_OVERRIDE_VMETHOD;
	bool applyCustomViewAttributeValues (CView* customView, IdStringPtr baseViewName, const UIAttributes& attributes, const IUIDescription* desc) const VSTGUI_OVERRIDE_VMETHOD;
	
	static void registerViewCreator (const IViewCreator& viewCreator);
	static void unregisterViewCreator (const IViewCreator& viewCreator);

#if VSTGUI_LIVE_EDITING
	typedef std::list<const std::string*> StringPtrList;
	typedef std::list<std::string> StringList;
	
	bool getAttributeNamesForView (CView* view, StringList& attributeNames) const;
	bool getAttributeValue (CView* view, const std::string& attributeName, std::string& stringValue, const IUIDescription* desc) const;
	IViewCreator::AttrType getAttributeType (CView* view, const std::string& attributeName) const;
	void collectRegisteredViewNames (StringPtrList& viewNames, IdStringPtr baseClassNameFilter = 0) const;
	bool getAttributesForView (CView* view, const IUIDescription* desc, UIAttributes& attr) const;
	// list type support
	bool getPossibleAttributeListValues (CView* view, const std::string& attributeName, StringPtrList& values) const;
	bool getAttributeValueRange (CView* view, const std::string& attributeName, double& minValue, double& maxValue) const;

#if ENABLE_UNIT_TESTS
	bool disableRememberAttributes {false};
#endif
#endif

protected:
	void evaluateAttributesAndRemember (CView* view, const UIAttributes& attributes, UIAttributes& evaluatedAttributes, const IUIDescription* description) const;
	CView* createViewByName (const std::string* className, const UIAttributes& attributes, const IUIDescription* description) const;

#if VSTGUI_LIVE_EDITING
	static size_t createHash (const std::string& str);
	void rememberAttribute (CView* view, IdStringPtr attrName, const std::string& value) const;
	bool getRememberedAttribute (CView* view, IdStringPtr attrName, std::string& value) const;
#endif
};

} // namespace

#endif // __uiviewfactory__

