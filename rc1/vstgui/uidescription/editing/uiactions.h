//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework not only for VST plugins :
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

#ifndef __uiactions__
#define __uiactions__

#include "iaction.h"

#if VSTGUI_LIVE_EDITING

#include "uiselection.h"
#include "../uiviewfactory.h"
#include "../../lib/ccolor.h"
#include "../../lib/cgradient.h"
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
template <class T>
class BaseSelectionOperation : public IAction, protected std::list<T>
{
public:
	BaseSelectionOperation (UISelection* selection) : selection (selection) {}

protected:
	SharedPointer<UISelection> selection;	
};

//-----------------------------------------------------------------------------
class SizeToFitOperation : public BaseSelectionOperation<std::pair<SharedPointer<CView>, CRect> >
{
public:
	SizeToFitOperation (UISelection* selection);
	~SizeToFitOperation ();

	UTF8StringPtr getName () VSTGUI_OVERRIDE_VMETHOD;
	
	void perform () VSTGUI_OVERRIDE_VMETHOD;
	void undo () VSTGUI_OVERRIDE_VMETHOD;
};

//-----------------------------------------------------------------------------
class UnembedViewOperation : public BaseSelectionOperation<SharedPointer<CView> >
{
public:
	UnembedViewOperation (UISelection* selection, const IViewFactory* factory);
	
	~UnembedViewOperation ();

	UTF8StringPtr getName () VSTGUI_OVERRIDE_VMETHOD;

	void perform () VSTGUI_OVERRIDE_VMETHOD;
	void undo () VSTGUI_OVERRIDE_VMETHOD;

protected:
	void collectSubviews (CViewContainer* container, bool deep);
	const IViewFactory* factory;
	SharedPointer<CViewContainer> containerView;
	CViewContainer* parent;
};

//-----------------------------------------------------------------------------
class EmbedViewOperation : public BaseSelectionOperation<std::pair<SharedPointer<CView>, CRect> >
{
public:
	EmbedViewOperation (UISelection* selection, CViewContainer* newContainer);
	~EmbedViewOperation ();
	
	UTF8StringPtr getName () VSTGUI_OVERRIDE_VMETHOD;
	void perform () VSTGUI_OVERRIDE_VMETHOD;
	void undo () VSTGUI_OVERRIDE_VMETHOD;

protected:
	OwningPointer<CViewContainer> newContainer;
	CViewContainer* parent;
};

//-----------------------------------------------------------------------------
class ViewCopyOperation : public IAction, protected std::list<SharedPointer<CView> >
{
public:
	ViewCopyOperation (UISelection* copySelection, UISelection* workingSelection, CViewContainer* parent, const CPoint& offset, IUIDescription* desc);
	~ViewCopyOperation ();
	
	UTF8StringPtr getName () VSTGUI_OVERRIDE_VMETHOD;
	void perform () VSTGUI_OVERRIDE_VMETHOD;
	void undo () VSTGUI_OVERRIDE_VMETHOD;
protected:
	SharedPointer<CViewContainer> parent;
	SharedPointer<UISelection> copySelection;
	SharedPointer<UISelection> workingSelection;
	std::list<SharedPointer<CView> > oldSelectedViews;
};

//-----------------------------------------------------------------------------
class ViewSizeChangeOperation : public BaseSelectionOperation<std::pair<SharedPointer<CView>, CRect> >
{
public:
	ViewSizeChangeOperation (UISelection* selection, bool sizing, bool autosizingEnabled);

	~ViewSizeChangeOperation ();
	
	UTF8StringPtr getName () VSTGUI_OVERRIDE_VMETHOD;
	void perform () VSTGUI_OVERRIDE_VMETHOD;
	void undo () VSTGUI_OVERRIDE_VMETHOD;
protected:
	bool first;
	bool sizing;
	bool autosizing;
};

//----------------------------------------------------------------------------------------------------
struct DeleteOperationViewAndNext
{
	DeleteOperationViewAndNext (CView* view, CView* nextView) : view (view), nextView (nextView) {}
	DeleteOperationViewAndNext (const DeleteOperationViewAndNext& copy) : view (copy.view), nextView (copy.nextView) {}
	SharedPointer<CView> view;
	SharedPointer<CView> nextView;
};

//----------------------------------------------------------------------------------------------------
class DeleteOperation : public IAction, protected std::multimap<SharedPointer<CViewContainer>, DeleteOperationViewAndNext*>
{
public:
	DeleteOperation (UISelection* selection);
	~DeleteOperation ();
	
	UTF8StringPtr getName () VSTGUI_OVERRIDE_VMETHOD;
	void perform () VSTGUI_OVERRIDE_VMETHOD;
	void undo () VSTGUI_OVERRIDE_VMETHOD;
protected:
	SharedPointer<UISelection> selection;
};

//-----------------------------------------------------------------------------
class InsertViewOperation : public IAction
{
public:
	InsertViewOperation (CViewContainer* parent, CView* view, UISelection* selection);
	~InsertViewOperation ();

	UTF8StringPtr getName () VSTGUI_OVERRIDE_VMETHOD;
	void perform () VSTGUI_OVERRIDE_VMETHOD;
	void undo () VSTGUI_OVERRIDE_VMETHOD;
protected:
	SharedPointer<CViewContainer> parent;
	SharedPointer<CView> view;
	SharedPointer<UISelection> selection;
};

//-----------------------------------------------------------------------------
class TransformViewTypeOperation : public IAction
{
public:
	TransformViewTypeOperation (UISelection* selection, IdStringPtr viewClassName, UIDescription* desc, const UIViewFactory* factory);
	~TransformViewTypeOperation ();

	UTF8StringPtr getName () VSTGUI_OVERRIDE_VMETHOD;

	void exchangeSubViews (CViewContainer* src, CViewContainer* dst);
	void perform () VSTGUI_OVERRIDE_VMETHOD;
	void undo () VSTGUI_OVERRIDE_VMETHOD;
protected:
	SharedPointer<CView> view;
	CView* newView;
	SharedPointer<CView> beforeView;
	SharedPointer<CViewContainer> parent;
	SharedPointer<UISelection> selection;
	const UIViewFactory* factory;
	SharedPointer<UIDescription> description;
};

//-----------------------------------------------------------------------------
class AttributeChangeAction : public IAction, protected std::map<SharedPointer<CView>, std::string>
{
public:
	AttributeChangeAction (UIDescription* desc, UISelection* selection, const std::string& attrName, const std::string& attrValue);

	~AttributeChangeAction ();

	UTF8StringPtr getName () VSTGUI_OVERRIDE_VMETHOD;
	void perform () VSTGUI_OVERRIDE_VMETHOD;
	void undo () VSTGUI_OVERRIDE_VMETHOD;
protected:
	void updateSelection ();
	
	UIDescription* desc;
	SharedPointer<UISelection> selection;
	std::string attrName;
	std::string attrValue;
	std::string name;
};

//----------------------------------------------------------------------------------------------------
class MultipleAttributeChangeAction : public IAction, public std::vector<std::pair<SharedPointer<CView>, std::string> >
{
public:
	MultipleAttributeChangeAction (UIDescription* description, const std::list<CView*>& views, IViewCreator::AttrType attrType, UTF8StringPtr oldValue, UTF8StringPtr newValue);
	virtual UTF8StringPtr getName () VSTGUI_OVERRIDE_VMETHOD { return "multiple view attribute changes"; }
	virtual void perform () VSTGUI_OVERRIDE_VMETHOD;
	virtual void undo () VSTGUI_OVERRIDE_VMETHOD;
protected:
	void setAttributeValue (UTF8StringPtr value);
	static void collectAllSubViews (CView* view, std::list<CView*>& views);
	void collectViewsWithAttributeValue (const UIViewFactory* viewFactory, IUIDescription* desc, CView* startView, IViewCreator::AttrType type, const std::string& value);

	SharedPointer<UIDescription> description;
	std::string oldValue;
	std::string newValue;
};

//----------------------------------------------------------------------------------------------------
class TagChangeAction : public IAction
{
public:
	TagChangeAction (UIDescription* description, UTF8StringPtr name, UTF8StringPtr newTagString, bool remove, bool performOrUndo);

	virtual UTF8StringPtr getName () VSTGUI_OVERRIDE_VMETHOD;
	virtual void perform () VSTGUI_OVERRIDE_VMETHOD;
	virtual void undo () VSTGUI_OVERRIDE_VMETHOD;
	
	bool isAddTag () const { return isNewTag; }
protected:
	SharedPointer<UIDescription> description;
	std::string name;
	std::string newTag;
	std::string originalTag;
	bool remove;
	bool performOrUndo;
	bool isNewTag;
};

//----------------------------------------------------------------------------------------------------
class TagNameChangeAction : public IAction
{
public:
	TagNameChangeAction (UIDescription* description, UTF8StringPtr oldName, UTF8StringPtr newName, bool performOrUndo);

	virtual UTF8StringPtr getName () VSTGUI_OVERRIDE_VMETHOD;
	virtual void perform () VSTGUI_OVERRIDE_VMETHOD;
	virtual void undo () VSTGUI_OVERRIDE_VMETHOD;
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

	virtual UTF8StringPtr getName () VSTGUI_OVERRIDE_VMETHOD;
	virtual void perform () VSTGUI_OVERRIDE_VMETHOD;
	virtual void undo () VSTGUI_OVERRIDE_VMETHOD;
	
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

	virtual UTF8StringPtr getName () VSTGUI_OVERRIDE_VMETHOD;
	virtual void perform () VSTGUI_OVERRIDE_VMETHOD;
	virtual void undo () VSTGUI_OVERRIDE_VMETHOD;
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

	virtual UTF8StringPtr getName () VSTGUI_OVERRIDE_VMETHOD;
	virtual void perform () VSTGUI_OVERRIDE_VMETHOD;
	virtual void undo () VSTGUI_OVERRIDE_VMETHOD;
	
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

	virtual UTF8StringPtr getName () VSTGUI_OVERRIDE_VMETHOD;
	virtual void perform () VSTGUI_OVERRIDE_VMETHOD;
	virtual void undo () VSTGUI_OVERRIDE_VMETHOD;
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
	
	virtual UTF8StringPtr getName () VSTGUI_OVERRIDE_VMETHOD;
	virtual void perform () VSTGUI_OVERRIDE_VMETHOD;
	virtual void undo () VSTGUI_OVERRIDE_VMETHOD;
protected:
	SharedPointer<UIDescription> description;
	std::string name;
	CRect* oldRect;
	CRect* newRect;
	bool performOrUndo;
};

//----------------------------------------------------------------------------------------------------
class BitmapFilterChangeAction : public IAction
{
public:
	BitmapFilterChangeAction (UIDescription* description, UTF8StringPtr bitmapName, const std::list<SharedPointer<UIAttributes> >& attributes, bool performOrUndo);
	~BitmapFilterChangeAction ();
	
	virtual UTF8StringPtr getName () VSTGUI_OVERRIDE_VMETHOD;
	virtual void perform () VSTGUI_OVERRIDE_VMETHOD;
	virtual void undo () VSTGUI_OVERRIDE_VMETHOD;
protected:
	SharedPointer<UIDescription> description;
	std::string bitmapName;
	std::list<SharedPointer<UIAttributes> > newAttributes;
	std::list<SharedPointer<UIAttributes> > oldAttributes;
	bool performOrUndo;
};

//----------------------------------------------------------------------------------------------------
class GradientChangeAction : public IAction
{
public:
	GradientChangeAction (UIDescription* description, UTF8StringPtr name, CGradient* gradient, bool remove, bool performOrUndo);

	virtual UTF8StringPtr getName () VSTGUI_OVERRIDE_VMETHOD;
	virtual void perform () VSTGUI_OVERRIDE_VMETHOD;
	virtual void undo () VSTGUI_OVERRIDE_VMETHOD;
	
	bool isAddGradient () const { return originalGradient == 0; }
protected:
	SharedPointer<UIDescription> description;
	std::string name;
	SharedPointer<CGradient> gradient;
	SharedPointer<CGradient> originalGradient;
	bool remove;
	bool performOrUndo;
};

//----------------------------------------------------------------------------------------------------
class GradientNameChangeAction : public IAction
{
public:
	GradientNameChangeAction (UIDescription* description, UTF8StringPtr oldName, UTF8StringPtr newName, bool performOrUndo);

	virtual UTF8StringPtr getName () VSTGUI_OVERRIDE_VMETHOD;
	virtual void perform () VSTGUI_OVERRIDE_VMETHOD;
	virtual void undo () VSTGUI_OVERRIDE_VMETHOD;
protected:
	SharedPointer<UIDescription> description;
	std::string oldName;
	std::string newName;
	bool performOrUndo;
};

//----------------------------------------------------------------------------------------------------
class FontChangeAction : public IAction
{
public:
	FontChangeAction (UIDescription* description, UTF8StringPtr name, CFontRef font, bool remove, bool performOrUndo);

	virtual UTF8StringPtr getName () VSTGUI_OVERRIDE_VMETHOD;
	virtual void perform () VSTGUI_OVERRIDE_VMETHOD;
	virtual void undo () VSTGUI_OVERRIDE_VMETHOD;

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

	virtual UTF8StringPtr getName () VSTGUI_OVERRIDE_VMETHOD;
	virtual void perform () VSTGUI_OVERRIDE_VMETHOD;
	virtual void undo () VSTGUI_OVERRIDE_VMETHOD;
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
	virtual UTF8StringPtr getName () VSTGUI_OVERRIDE_VMETHOD;
	virtual void perform () VSTGUI_OVERRIDE_VMETHOD;
	virtual void undo () VSTGUI_OVERRIDE_VMETHOD;
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

	UTF8StringPtr getName () VSTGUI_OVERRIDE_VMETHOD;
	void perform () VSTGUI_OVERRIDE_VMETHOD;
	void undo () VSTGUI_OVERRIDE_VMETHOD;
protected:
	SharedPointer<CView> view;
	SharedPointer<CViewContainer> parent;
	SharedPointer<UISelection> selection;
	bool up;
};

//-----------------------------------------------------------------------------
class TemplateNameChangeAction : public IAction
{
public:
	TemplateNameChangeAction (UIDescription* description, IActionPerformer* actionPerformer, UTF8StringPtr oldName, UTF8StringPtr newName);

	UTF8StringPtr getName () VSTGUI_OVERRIDE_VMETHOD;
	void perform () VSTGUI_OVERRIDE_VMETHOD;
	void undo () VSTGUI_OVERRIDE_VMETHOD;
protected:
	SharedPointer<UIDescription> description;
	IActionPerformer* actionPerformer;
	std::string oldName;
	std::string newName;
};

//-----------------------------------------------------------------------------
class CreateNewTemplateAction : public IAction
{
public:
	CreateNewTemplateAction (UIDescription* description, IActionPerformer* actionPerformer, UTF8StringPtr name, UTF8StringPtr baseViewClassName);

	UTF8StringPtr getName () VSTGUI_OVERRIDE_VMETHOD;
	void perform () VSTGUI_OVERRIDE_VMETHOD;
	void undo () VSTGUI_OVERRIDE_VMETHOD;
protected:
	SharedPointer<UIDescription> description;
	IActionPerformer* actionPerformer;
	SharedPointer<CView> view;
	std::string name;
	std::string baseViewClassName;
};

//-----------------------------------------------------------------------------
class DuplicateTemplateAction : public IAction
{
public:
	DuplicateTemplateAction (UIDescription* description, IActionPerformer* actionPerformer, UTF8StringPtr name, UTF8StringPtr dupName);

	UTF8StringPtr getName () VSTGUI_OVERRIDE_VMETHOD;
	void perform () VSTGUI_OVERRIDE_VMETHOD;
	void undo () VSTGUI_OVERRIDE_VMETHOD;
protected:
	SharedPointer<UIDescription> description;
	IActionPerformer* actionPerformer;
	SharedPointer<CView> view;
	std::string name;
	std::string dupName;
};

//-----------------------------------------------------------------------------
class DeleteTemplateAction : public IAction
{
public:
	DeleteTemplateAction (UIDescription* description, IActionPerformer* actionPerformer, CView* view, UTF8StringPtr name);

	UTF8StringPtr getName () VSTGUI_OVERRIDE_VMETHOD;
	void perform () VSTGUI_OVERRIDE_VMETHOD;
	void undo () VSTGUI_OVERRIDE_VMETHOD;
protected:
	SharedPointer<UIDescription> description;
	IActionPerformer* actionPerformer;
	SharedPointer<CView> view;
	SharedPointer<UIAttributes> attributes;
	std::string name;
};

} // namespace

#endif // VSTGUI_LIVE_EDITING

#endif // __uiactions__
