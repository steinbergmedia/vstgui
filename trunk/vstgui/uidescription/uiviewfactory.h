//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 4.2
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2013, Steinberg Media Technologies, All Rights Reserved
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

#include "../lib/cview.h"
#include "uidescription.h"
#include <string>
#include <list>

namespace VSTGUI {

//-----------------------------------------------------------------------------
/// @brief View creator interface
///	@ingroup new_in_4_0
//-----------------------------------------------------------------------------
class IViewCreator
{
public:
	virtual ~IViewCreator () {}
	
	enum AttrType {
		kUnknownType,
		kBooleanType,
		kIntegerType,
		kFloatType,
		kStringType,
		kColorType,
		kFontType,
		kBitmapType,
		kPointType,
		kRectType,
		kTagType,
		kListType
	};

	virtual IdStringPtr getViewName () const = 0;
	virtual IdStringPtr getBaseViewName () const = 0;
	virtual CView* create (const UIAttributes& attributes, IUIDescription* description) const = 0;
	virtual bool apply (CView* view, const UIAttributes& attributes, IUIDescription* description) const = 0;
	virtual bool getAttributeNames (std::list<std::string>& attributeNames) const = 0;
	virtual AttrType getAttributeType (const std::string& attributeName) const = 0;
	virtual bool getAttributeValue (CView* view, const std::string& attributeName, std::string& stringValue, IUIDescription* desc) const = 0;
	// list type support
	virtual bool getPossibleListValues (const std::string& attributeName, std::list<const std::string*>& values) const { return false; }
};

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
	CView* createView (const UIAttributes& attributes, IUIDescription* description) VSTGUI_OVERRIDE_VMETHOD;
	bool applyAttributeValues (CView* view, const UIAttributes& attributes, IUIDescription* desc) const VSTGUI_OVERRIDE_VMETHOD;
	
	static void registerViewCreator (const IViewCreator& viewCreator);

	#if VSTGUI_LIVE_EDITING
	bool getAttributeNamesForView (CView* view, std::list<std::string>& attributeNames) const;
	bool getAttributeValue (CView* view, const std::string& attributeName, std::string& stringValue, IUIDescription* desc) const;
	IViewCreator::AttrType getAttributeType (CView* view, const std::string& attributeName) const;
	void collectRegisteredViewNames (std::list<const std::string*>& viewNames, IdStringPtr baseClassNameFilter = 0) const;
	bool getAttributesForView (CView* view, IUIDescription* desc, UIAttributes& attr) const;
	// list type support
	bool getPossibleAttributeListValues (CView* view, const std::string& attributeName, std::list<const std::string*>& values) const;
	#endif

	IdStringPtr getViewName (CView* view) const;
	bool applyCustomViewAttributeValues (CView* customView, IdStringPtr baseViewName, const UIAttributes& attributes, IUIDescription* desc) const;

protected:
	CView* createViewByName (const std::string* className, const UIAttributes& attributes, IUIDescription* description);
};

} // namespace

#endif // __uiviewfactory__

