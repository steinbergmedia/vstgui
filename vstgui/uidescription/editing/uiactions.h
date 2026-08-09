// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "iaction.h"

#if VSTGUI_LIVE_EDITING

#include "uiselection.h"
#include "../iviewfactory.h"
#include "../iuidescription.h"
#include "../../lib/ccolor.h"
#include "../../lib/cgradient.h"
#include <list>
#include <map>
#include <vector>
#include <string>
#include <functional>

namespace VSTGUI {
class UIViewFactory;
class IUIDescription;
class UIDescription;
class CViewContainer;
class CView;

//-----------------------------------------------------------------------------
class Action : public IAction
{
public:
	using Func = std::function<void ()>;
	Action (const std::string& name, Func&& perform, Func&& undo)
	: name (name), performAction (std::move (perform)), undoAction (std::move (undo))
	{
	}

	UTF8StringPtr getName () override { return name.data (); }

	void perform () override { performAction (); }
	void undo () override { undoAction (); }

private:
	std::string name;
	Func performAction;
	Func undoAction;
};

//-----------------------------------------------------------------------------
template <class T>
class BaseSelectionOperation : public IAction, protected std::list<T>
{
public:
	BaseSelectionOperation (const SPtr<UISelection>& selection) : selection (selection) {}

protected:
	SPtr<UISelection> selection;
};

//-----------------------------------------------------------------------------
class SizeToFitOperation : public BaseSelectionOperation<std::pair<SPtr<CView>, CRect>>
{
public:
	SizeToFitOperation (const SPtr<UISelection>& selection);
	~SizeToFitOperation () override = default;

	UTF8StringPtr getName () override;
	
	void perform () override;
	void undo () override;
};

//-----------------------------------------------------------------------------
class UnembedViewOperation : public BaseSelectionOperation<SPtr<CView>>
{
public:
	UnembedViewOperation (const SPtr<UISelection>& selection, const IViewFactory& factory);
	~UnembedViewOperation () override = default;

	UTF8StringPtr getName () override;

	void perform () override;
	void undo () override;

protected:
	void collectSubviews (CViewContainer& container, bool deep);
	const IViewFactory& factory;
	SPtr<CViewContainer> containerView;
	SPtr<CViewContainer> parent;
};

//-----------------------------------------------------------------------------
class EmbedViewOperation : public BaseSelectionOperation<std::pair<SPtr<CView>, CRect>>
{
public:
	EmbedViewOperation (const SPtr<UISelection>& selection,
						const SPtr<CViewContainer>& newContainer);
	~EmbedViewOperation () override = default;
	
	UTF8StringPtr getName () override;
	void perform () override;
	void undo () override;

protected:
	SPtr<CViewContainer> newContainer;
	SPtr<CViewContainer> parent;
};

//-----------------------------------------------------------------------------
class ViewCopyOperation : public IAction,
						  protected std::list<SPtr<CView>>
{
public:
	ViewCopyOperation (const SPtr<UISelection>& copySelection,
					   const SPtr<UISelection>& workingSelection,
					   const SPtr<CViewContainer>& parent, const CPoint& offset,
					   const SPtr<IUIDescription>& desc);
	~ViewCopyOperation () override = default;
	
	UTF8StringPtr getName () override;
	void perform () override;
	void undo () override;
protected:
	SPtr<CViewContainer> parent;
	SPtr<UISelection> copySelection;
	SPtr<UISelection> workingSelection;
	std::list<SPtr<CView>> oldSelectedViews;
};

//-----------------------------------------------------------------------------
class ViewSizeChangeOperation : public BaseSelectionOperation<std::pair<SPtr<CView>, CRect>>
{
public:
	ViewSizeChangeOperation (const SPtr<UISelection>& selection, bool sizing,
							 bool autosizingEnabled);
	~ViewSizeChangeOperation () override = default;
	
	UTF8StringPtr getName () override;
	void perform () override;
	void undo () override;
	
	bool didChange ();
protected:
	bool first;
	bool sizing;
	bool autosizing;
};

//----------------------------------------------------------------------------------------------------
class DeleteOperation : public IAction
{
public:
	DeleteOperation (const SPtr<UISelection>& selection);
	~DeleteOperation () override = default;
	
	UTF8StringPtr getName () override;
	void perform () override;
	void undo () override;
protected:
	struct ViewAndNext
	{
		SPtr<CView> view;
		SPtr<CView> nextView;
	};

	SPtr<UISelection> selection;
	std::multimap<SPtr<CViewContainer>, ViewAndNext> map;
};

//-----------------------------------------------------------------------------
class InsertViewOperation : public IAction
{
public:
	InsertViewOperation (const SPtr<CViewContainer>& parent, const SPtr<CView>& view,
						 const SPtr<UISelection>& selection);
	~InsertViewOperation () override = default;

	UTF8StringPtr getName () override;
	void perform () override;
	void undo () override;
protected:
	SPtr<CViewContainer> parent;
	SPtr<CView> view;
	SPtr<UISelection> selection;
};

//-----------------------------------------------------------------------------
class TransformViewTypeOperation : public IAction
{
public:
	TransformViewTypeOperation (const SPtr<UISelection>& selection, const SPtr<CView>& view,
								IdStringPtr viewClassName, const SPtr<UIDescription>& desc,
								const IViewFactory& factory);
	~TransformViewTypeOperation () override;

	UTF8StringPtr getName () override;

	void perform () override;
	void undo () override;
protected:
	void exchangeSubViews (const SPtr<CViewContainer>& src, const SPtr<CViewContainer>& dst);

	SPtr<CView> view;
	SPtr<CView> newView;
	int32_t insertIndex;
	SPtr<CViewContainer> parent;
	SPtr<UISelection> selection;
	const IViewFactory& factory;
	SPtr<UIDescription> description;
};

//-----------------------------------------------------------------------------
class AttributeChangeAction : public IAction,
							  protected std::map<SPtr<CView>, std::string>
{
public:
	AttributeChangeAction (const SPtr<UIDescription>& desc, const SPtr<UISelection>& selection,
						   const std::string& attrName, const std::string& attrValue);
	~AttributeChangeAction () override = default;

	UTF8StringPtr getName () override;
	void perform () override;
	void undo () override;
protected:
	void updateSelection ();

	SPtr<UIDescription> desc;
	SPtr<UISelection> selection;
	std::string attrName;
	std::string attrValue;
	std::string name;
};

//----------------------------------------------------------------------------------------------------
class MultipleAttributeChangeAction : public IAction,
									  public std::vector<std::pair<SPtr<CView>, std::string>>
{
public:
	MultipleAttributeChangeAction (const SPtr<UIDescription>& description,
								   const std::list<SPtr<CView>>& views,
								   IViewCreator::AttrType attrType, UTF8StringPtr oldValue,
								   UTF8StringPtr newValue);
	UTF8StringPtr getName () override { return "multiple view attribute changes"; }
	void perform () override;
	void undo () override;
protected:
	void setAttributeValue (UTF8StringPtr value);
	static void collectAllSubViews (const SPtr<CView>& view, std::list<SPtr<CView>>& views);
	void collectViewsWithAttributeValue (const IViewFactory& viewFactory,
										 const SPtr<IUIDescription>& desc,
										 const SPtr<CView>& startView, IViewCreator::AttrType type,
										 const std::string& value);

	SPtr<UIDescription> description;
	std::string oldValue;
	std::string newValue;
};

//----------------------------------------------------------------------------------------------------
class TagChangeAction : public IAction
{
public:
	TagChangeAction (const SPtr<UIDescription>& description, UTF8StringPtr name,
					 UTF8StringPtr newTagString, bool remove, bool performOrUndo);

	UTF8StringPtr getName () override;
	void perform () override;
	void undo () override;
	
	bool isAddTag () const { return isNewTag; }
protected:
	SPtr<UIDescription> description;
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
	TagNameChangeAction (const SPtr<UIDescription>& description, UTF8StringPtr oldName,
						 UTF8StringPtr newName, bool performOrUndo);

	UTF8StringPtr getName () override;
	void perform () override;
	void undo () override;
protected:
	SPtr<UIDescription> description;
	std::string oldName;
	std::string newName;
	bool performOrUndo;
};

//----------------------------------------------------------------------------------------------------
class ColorChangeAction : public IAction
{
public:
	ColorChangeAction (const SPtr<UIDescription>& description, UTF8StringPtr name,
					   const CColor& color, bool remove, bool performOrUndo);

	UTF8StringPtr getName () override;
	void perform () override;
	void undo () override;
	
	bool isAddColor () const { return isNewColor; }
protected:
	SPtr<UIDescription> description;
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
	ColorNameChangeAction (const SPtr<UIDescription>& description, UTF8StringPtr oldName,
						   UTF8StringPtr newName, bool performOrUndo);

	UTF8StringPtr getName () override;
	void perform () override;
	void undo () override;
protected:
	SPtr<UIDescription> description;
	std::string oldName;
	std::string newName;
	bool performOrUndo;
};

//----------------------------------------------------------------------------------------------------
class BitmapChangeAction : public IAction
{
public:
	BitmapChangeAction (const SPtr<UIDescription>& description, UTF8StringPtr name,
						UTF8StringPtr path, bool remove, bool performOrUndo);

	UTF8StringPtr getName () override;
	void perform () override;
	void undo () override;
	
	bool isAddBitmap () const { return isNewBitmap; }
protected:
	SPtr<UIDescription> description;
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
	BitmapNameChangeAction (const SPtr<UIDescription>& description, UTF8StringPtr oldName,
							UTF8StringPtr newName, bool performOrUndo);

	UTF8StringPtr getName () override;
	void perform () override;
	void undo () override;
protected:
	SPtr<UIDescription> description;
	std::string oldName;
	std::string newName;
	bool performOrUndo;
};

//----------------------------------------------------------------------------------------------------
class NinePartTiledBitmapChangeAction : public IAction
{
public:
	NinePartTiledBitmapChangeAction (const SPtr<UIDescription>& description, UTF8StringPtr name,
									 const CRect* rect, bool performOrUndo);
	UTF8StringPtr getName () override;
	void perform () override;
	void undo () override;
protected:
	SPtr<UIDescription> description;
	std::string name;
	std::unique_ptr<CRect> oldRect;
	std::unique_ptr<CRect> newRect;
	bool performOrUndo;
};

//----------------------------------------------------------------------------------------------------
class MultiFrameBitmapChangeAction : public IAction
{
public:
	MultiFrameBitmapChangeAction (const SPtr<UIDescription>& description, UTF8StringPtr name,
								  const CMultiFrameBitmapDescription* desc, bool performOrUndo);
	~MultiFrameBitmapChangeAction () override;

	UTF8StringPtr getName () override;
	void perform () override;
	void undo () override;

protected:
	SPtr<UIDescription> description;
	std::string name;
	std::unique_ptr<CMultiFrameBitmapDescription> oldDesc;
	std::unique_ptr<CMultiFrameBitmapDescription> newDesc;
	bool performOrUndo;
};

//----------------------------------------------------------------------------------------------------
class BitmapFilterChangeAction : public IAction
{
public:
	BitmapFilterChangeAction (const SPtr<UIDescription>& description, UTF8StringPtr bitmapName,
							  const std::list<SPtr<UIAttributes>>& attributes, bool performOrUndo);
	~BitmapFilterChangeAction () override = default;
	
	UTF8StringPtr getName () override;
	void perform () override;
	void undo () override;
protected:
	SPtr<UIDescription> description;
	std::string bitmapName;
	std::list<SPtr<UIAttributes>> newAttributes;
	std::list<SPtr<UIAttributes>> oldAttributes;
	bool performOrUndo;
};

//----------------------------------------------------------------------------------------------------
class GradientChangeAction : public IAction
{
public:
	GradientChangeAction (const SPtr<UIDescription>& description, UTF8StringPtr name,
						  const SPtr<CGradient>& gradient, bool remove, bool performOrUndo);

	UTF8StringPtr getName () override;
	void perform () override;
	void undo () override;
	
	bool isAddGradient () const { return originalGradient == 0; }
protected:
	SPtr<UIDescription> description;
	std::string name;
	SPtr<CGradient> gradient;
	SPtr<CGradient> originalGradient;
	bool remove;
	bool performOrUndo;
};

//----------------------------------------------------------------------------------------------------
class GradientNameChangeAction : public IAction
{
public:
	GradientNameChangeAction (const SPtr<UIDescription>& description, UTF8StringPtr oldName,
							  UTF8StringPtr newName, bool performOrUndo);

	UTF8StringPtr getName () override;
	void perform () override;
	void undo () override;
protected:
	SPtr<UIDescription> description;
	std::string oldName;
	std::string newName;
	bool performOrUndo;
};

//----------------------------------------------------------------------------------------------------
class FontChangeAction : public IAction
{
public:
	FontChangeAction (const SPtr<UIDescription>& description, UTF8StringPtr name,
					  const SPtr<CFontDesc>& font, bool remove, bool performOrUndo);

	UTF8StringPtr getName () override;
	void perform () override;
	void undo () override;

	bool isAddFont () const { return originalFont == 0; }
protected:
	SPtr<UIDescription> description;
	std::string name;
	std::string alternativeNames;
	SPtr<CFontDesc> font;
	SPtr<CFontDesc> originalFont;
	bool remove;
	bool performOrUndo;
};

//----------------------------------------------------------------------------------------------------
class FontNameChangeAction : public IAction
{
public:
	FontNameChangeAction (const SPtr<UIDescription>& description, UTF8StringPtr oldName,
						  UTF8StringPtr newName, bool performOrUndo);

	UTF8StringPtr getName () override;
	void perform () override;
	void undo () override;
protected:
	SPtr<UIDescription> description;
	std::string oldName;
	std::string newName;
	bool performOrUndo;
};

//-----------------------------------------------------------------------------
class AlternateFontChangeAction : public IAction
{
public:
	AlternateFontChangeAction (const SPtr<UIDescription>& description, UTF8StringPtr fontName,
							   UTF8StringPtr newAlternateFontNames);
	UTF8StringPtr getName () override;
	void perform () override;
	void undo () override;
protected:
	SPtr<UIDescription> description;
	std::string fontName;
	std::string newAlternateFontNames;
	std::string oldAlternateFontNames;
};

//-----------------------------------------------------------------------------
class HierarchyMoveViewOperation : public IAction
{
public:
	HierarchyMoveViewOperation (const SPtr<CView>& view, const SPtr<UISelection>& selection,
								int32_t dir);
	~HierarchyMoveViewOperation () override = default;

	UTF8StringPtr getName () override;
	void perform () override;
	void undo () override;
protected:
	SPtr<CView> view;
	SPtr<CViewContainer> parent;
	SPtr<UISelection> selection;
	int32_t dir;
};

//-----------------------------------------------------------------------------
class TemplateNameChangeAction : public IAction
{
public:
	TemplateNameChangeAction (const SPtr<UIDescription>& description,
							  WeakPointer<IActionPerformer> actionPerformer, UTF8StringPtr oldName,
							  UTF8StringPtr newName);

	UTF8StringPtr getName () override;
	void perform () override;
	void undo () override;
protected:
	SPtr<UIDescription> description;
	WeakPointer<IActionPerformer> actionPerformer;
	std::string oldName;
	std::string newName;
};

//-----------------------------------------------------------------------------
class CreateNewTemplateAction : public IAction
{
public:
	CreateNewTemplateAction (const SPtr<UIDescription>& description,
							 WeakPointer<IActionPerformer> actionPerformer, UTF8StringPtr name,
							 UTF8StringPtr baseViewClassName);

	UTF8StringPtr getName () override;
	void perform () override;
	void undo () override;
protected:
	SPtr<UIDescription> description;
	WeakPointer<IActionPerformer> actionPerformer;
	SPtr<CView> view;
	std::string name;
	std::string baseViewClassName;
};

//-----------------------------------------------------------------------------
class DuplicateTemplateAction : public IAction
{
public:
	DuplicateTemplateAction (const SPtr<UIDescription>& description,
							 WeakPointer<IActionPerformer> actionPerformer, UTF8StringPtr name,
							 UTF8StringPtr dupName);

	UTF8StringPtr getName () override;
	void perform () override;
	void undo () override;
protected:
	SPtr<UIDescription> description;
	WeakPointer<IActionPerformer> actionPerformer;
	SPtr<CView> view;
	std::string name;
	std::string dupName;
};

//-----------------------------------------------------------------------------
class DeleteTemplateAction : public IAction
{
public:
	DeleteTemplateAction (const SPtr<UIDescription>& description,
						  WeakPointer<IActionPerformer> actionPerformer, const SPtr<CView>& view,
						  UTF8StringPtr name);

	UTF8StringPtr getName () override;
	void perform () override;
	void undo () override;
protected:
	SPtr<UIDescription> description;
	WeakPointer<IActionPerformer> actionPerformer;
	SPtr<CView> view;
	SPtr<UIAttributes> attributes;
	std::string name;
};

//-----------------------------------------------------------------------------
class ChangeFocusDrawingAction : public IAction
{
public:
	ChangeFocusDrawingAction (const SPtr<UIDescription>& description,
							  const FocusDrawingSettings& newSettings);

	UTF8StringPtr getName () override;
	void perform () override;
	void undo () override;
protected:
	SPtr<UIDescription> description;
	FocusDrawingSettings oldSettings;
	FocusDrawingSettings newSettings;
};

//-----------------------------------------------------------------------------
class ChangeTemplateMinMaxAction : public IAction
{
public:
	ChangeTemplateMinMaxAction (const SPtr<UIDescription>& description, UTF8StringPtr templateName,
								CPoint minSize, CPoint maxSize);

	UTF8StringPtr getName () override;
	void perform () override;
	void undo () override;
private:
	void setMinMaxSize (CPoint minimum, CPoint maximum);

	SPtr<UIDescription> description;
	std::string templateName;
	CPoint minSize;
	CPoint maxSize;
	CPoint oldMinSize;
	CPoint oldMaxSize;
};

//-----------------------------------------------------------------------------
} // VSTGUI

#endif // VSTGUI_LIVE_EDITING
