//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework not only for VST plugins :
//
// Version 4.1
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2011, Steinberg Media Technologies, All Rights Reserved
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
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A  PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#ifndef __uiactions__
#define __uiactions__

#include "iaction.h"

#if VSTGUI_LIVE_EDITING

#include "uiselection.h"
#include "../uiviewfactory.h"
#include <list>
#include <map>
#include <string>

namespace VSTGUI {
class UIViewFactory;
class IUIDescription;
class UIDescription;
class CViewContainer;
class CView;

//-----------------------------------------------------------------------------
class SizeToFitOperation : public IAction, protected std::list<CView*>
{
public:
	SizeToFitOperation (UISelection* selection);
	~SizeToFitOperation ();

	UTF8StringPtr getName ();
	
	void perform ();
	void undo ();

protected:
	UISelection* selection;
	std::list<CRect> sizes;
};

//-----------------------------------------------------------------------------
class UnembedViewOperation : public IAction, protected std::list<CView*>
{
public:
	UnembedViewOperation (UISelection* selection, UIViewFactory* factory);
	
	~UnembedViewOperation ();

	UTF8StringPtr getName ();

	void perform ();
	void undo ();

protected:
	UIViewFactory* factory;
	UISelection* selection;
	CViewContainer* containerView;
	CViewContainer* parent;
};

//-----------------------------------------------------------------------------
class EmbedViewOperation : public IAction, protected std::list<CView*>
{
public:
	EmbedViewOperation (UISelection* selection, CViewContainer* newContainer);
	~EmbedViewOperation ();
	
	UTF8StringPtr getName ();
	void perform ();
	void undo ();

protected:
	UISelection* selection;
	CViewContainer* newContainer;
	CViewContainer* parent;
};

//-----------------------------------------------------------------------------
class ViewCopyOperation : public IAction, protected std::list<CView*>
{
public:
	ViewCopyOperation (UISelection* copySelection, UISelection* workingSelection, CViewContainer* parent, const CPoint& offset, UIViewFactory* viewFactory, IUIDescription* desc);
	~ViewCopyOperation ();
	
	UTF8StringPtr getName ();
	void perform ();
	void undo ();
protected:
	CViewContainer* parent;
	UISelection* copySelection;
	UISelection* workingSelection;
	std::list<CView*> oldSelectedViews;
};

//-----------------------------------------------------------------------------
class ViewSizeChangeOperation : public IAction, protected std::map<CView*, CRect>
{
public:
	ViewSizeChangeOperation (UISelection* selection, bool sizing);

	~ViewSizeChangeOperation ();
	
	UTF8StringPtr getName ();
	void perform ();
	void undo ();
protected:
	UISelection* selection;

	bool first;
	bool sizing;
};

//----------------------------------------------------------------------------------------------------
struct DeleteOperationViewAndNext
{
	DeleteOperationViewAndNext (CView* view, CView* nextView) : view (view), nextView (nextView) {}
	DeleteOperationViewAndNext (const DeleteOperationViewAndNext& copy) : view (copy.view), nextView (copy.nextView) {}
	CView* view;
	CView* nextView;
};

//----------------------------------------------------------------------------------------------------
class DeleteOperation : public IAction, protected std::multimap<CViewContainer*, DeleteOperationViewAndNext*>
{
public:
	DeleteOperation (UISelection* selection);
	~DeleteOperation ();
	
	UTF8StringPtr getName ();
	void perform ();
	void undo ();
protected:
	UISelection* selection;
};

//-----------------------------------------------------------------------------
class InsertViewOperation : public IAction
{
public:
	InsertViewOperation (CViewContainer* parent, CView* view, UISelection* selection);
	~InsertViewOperation ();

	UTF8StringPtr getName ();
	void perform ();
	void undo ();
protected:
	CViewContainer* parent;
	CView* view;
	UISelection* selection;
};

//-----------------------------------------------------------------------------
class TransformViewTypeOperation : public IAction
{
public:
	TransformViewTypeOperation (UISelection* selection, IdStringPtr viewClassName, IUIDescription* desc, UIViewFactory* factory);
	~TransformViewTypeOperation ();

	UTF8StringPtr getName ();

	void exchangeSubViews (CViewContainer* src, CViewContainer* dst);
	void perform ();
	void undo ();
protected:
	CView* view;
	CView* newView;
	CView* beforeView;
	CViewContainer* parent;
	UISelection* selection;
};

//-----------------------------------------------------------------------------
class AttributeChangeAction : public IAction, protected std::map<CView*, std::string>
{
public:
	AttributeChangeAction (UIDescription* desc, UISelection* selection, const std::string& attrName, const std::string& attrValue);

	~AttributeChangeAction ();

	UTF8StringPtr getName ();
	void perform ();
	void undo ();
protected:
	UIDescription* desc;
	UISelection* selection;
	std::string attrName;
	std::string attrValue;
	std::string name;
};

//----------------------------------------------------------------------------------------------------
class MultipleAttributeChangeAction : public IAction, public std::map<SharedPointer<CView>, std::string>
{
public:
	MultipleAttributeChangeAction (UIDescription* description, CView* baseView, IViewCreator::AttrType attrType, UTF8StringPtr oldValue, UTF8StringPtr newValue);
	virtual UTF8StringPtr getName () { return "multiple view attribute changes"; }
	virtual void perform ();
	virtual void undo ();
protected:
	void setAttributeValue (UTF8StringPtr value);
	static void collectAllSubViews (CView* view, std::list<CView*>& views);
	void collectViewsWithAttributeValue (UIViewFactory* viewFactory, IUIDescription* desc, CView* startView, IViewCreator::AttrType type, const std::string& value);

	SharedPointer<UIDescription> description;
	SharedPointer<CView> baseView;
	std::string oldValue;
	std::string newValue;
};

//----------------------------------------------------------------------------------------------------
class TagChangeAction : public IAction
{
public:
	TagChangeAction (UIDescription* description, UTF8StringPtr name, int32_t tag, bool remove, bool performOrUndo);

	virtual UTF8StringPtr getName ();
	virtual void perform ();
	virtual void undo ();
	
	bool isAddTag () const { return isNewTag; }
protected:
	SharedPointer<UIDescription> description;
	std::string name;
	int32_t tag;
	int32_t originalTag;
	bool remove;
	bool performOrUndo;
	bool isNewTag;
};

//----------------------------------------------------------------------------------------------------
class TagNameChangeAction : public IAction
{
public:
	TagNameChangeAction (UIDescription* description, UTF8StringPtr oldName, UTF8StringPtr newName, bool performOrUndo);

	virtual UTF8StringPtr getName ();
	virtual void perform ();
	virtual void undo ();
protected:
	SharedPointer<UIDescription> description;
	std::string oldName;
	std::string newName;
	bool performOrUndo;
};

//----------------------------------------------------------------------------------------------------
class ColorChangeAction : public IAction
{
public:
	ColorChangeAction (UIDescription* description, UTF8StringPtr name, const CColor& color, bool remove, bool performOrUndo);

	virtual UTF8StringPtr getName ();
	virtual void perform ();
	virtual void undo ();
	
	bool isAddColor () const { return isNewColor; }
protected:
	SharedPointer<UIDescription> description;
	std::string name;
	CColor newColor;
	CColor oldColor;
	bool remove;
	bool performOrUndo;
	bool isNewColor;
};

//----------------------------------------------------------------------------------------------------
class ColorNameChangeAction : public IAction
{
public:
	ColorNameChangeAction (UIDescription* description, UTF8StringPtr oldName, UTF8StringPtr newName, bool performOrUndo);

	virtual UTF8StringPtr getName ();
	virtual void perform ();
	virtual void undo ();
protected:
	SharedPointer<UIDescription> description;
	std::string oldName;
	std::string newName;
	bool performOrUndo;
};

//----------------------------------------------------------------------------------------------------
class BitmapChangeAction : public IAction
{
public:
	BitmapChangeAction (UIDescription* description, UTF8StringPtr name, UTF8StringPtr path, bool remove, bool performOrUndo);

	virtual UTF8StringPtr getName ();
	virtual void perform ();
	virtual void undo ();
	
	bool isAddBitmap () const { return isNewBitmap; }
protected:
	SharedPointer<UIDescription> description;
	std::string name;
	std::string path;
	std::string originalPath;
	bool remove;
	bool performOrUndo;
	bool isNewBitmap;
};

//----------------------------------------------------------------------------------------------------
class BitmapNameChangeAction : public IAction
{
public:
	BitmapNameChangeAction (UIDescription* description, UTF8StringPtr oldName, UTF8StringPtr newName, bool performOrUndo);

	virtual UTF8StringPtr getName ();
	virtual void perform ();
	virtual void undo ();
protected:
	SharedPointer<UIDescription> description;
	std::string oldName;
	std::string newName;
	bool performOrUndo;
};

//----------------------------------------------------------------------------------------------------
class NinePartTiledBitmapChangeAction : public IAction
{
public:
	NinePartTiledBitmapChangeAction (UIDescription* description, UTF8StringPtr name, const CRect* rect, bool performOrUndo);
	~NinePartTiledBitmapChangeAction ();
	
	virtual UTF8StringPtr getName ();
	virtual void perform ();
	virtual void undo ();
protected:
	SharedPointer<UIDescription> description;
	std::string name;
	CRect* oldRect;
	CRect* newRect;
	bool performOrUndo;
};

//----------------------------------------------------------------------------------------------------
class FontChangeAction : public IAction
{
public:
	FontChangeAction (UIDescription* description, UTF8StringPtr name, CFontRef font, bool remove, bool performOrUndo);

	virtual UTF8StringPtr getName ();
	virtual void perform ();
	virtual void undo ();

	bool isAddFont () const { return originalFont == 0; }
protected:
	SharedPointer<UIDescription> description;
	std::string name;
	std::string alternativeNames;
	SharedPointer<CFontDesc> font;
	SharedPointer<CFontDesc> originalFont;
	bool remove;
	bool performOrUndo;
};

//----------------------------------------------------------------------------------------------------
class FontNameChangeAction : public IAction
{
public:
	FontNameChangeAction (UIDescription* description, UTF8StringPtr oldName, UTF8StringPtr newName, bool performOrUndo);

	virtual UTF8StringPtr getName ();
	virtual void perform ();
	virtual void undo ();
protected:
	SharedPointer<UIDescription> description;
	std::string oldName;
	std::string newName;
	bool performOrUndo;
};

//-----------------------------------------------------------------------------
class AlternateFontChangeAction : public IAction
{
public:
	AlternateFontChangeAction (UIDescription* description, UTF8StringPtr fontName, UTF8StringPtr newAlternateFontNames);
	virtual UTF8StringPtr getName ();
	virtual void perform ();
	virtual void undo ();
protected:
	SharedPointer<UIDescription> description;
	std::string fontName;
	std::string newAlternateFontNames;
	std::string oldAlternateFontNames;
};

//-----------------------------------------------------------------------------
class HierarchyMoveViewOperation : public IAction
{
public:
	HierarchyMoveViewOperation (CView* view, UISelection* selection, bool up);
	~HierarchyMoveViewOperation ();

	UTF8StringPtr getName ();
	void perform ();
	void undo ();
protected:
	SharedPointer<CView> view;
	SharedPointer<CViewContainer> parent;
	UISelection* selection;
	bool up;
};

//-----------------------------------------------------------------------------
class TemplateNameChangeAction : public IAction
{
public:
	TemplateNameChangeAction (UIDescription* description, UTF8StringPtr oldName, UTF8StringPtr newName);

	UTF8StringPtr getName ();
	void perform ();
	void undo ();
protected:
	SharedPointer<UIDescription> description;
	std::string oldName;
	std::string newName;
};

} // namespace

#endif // VSTGUI_LIVE_EDITING

#endif // __uiactions__
