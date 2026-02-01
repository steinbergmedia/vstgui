// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "cfontchooser.h"
#include "../cdatabrowser.h"
#include "../cdrawcontext.h"
#include "cbuttons.h"
#include "ctextedit.h"
#include "cscrollbar.h"
#include "../cstring.h"
#include "../platform/platformfactory.h"
#include "../platform/iplatformfont.h"
#include <list>
#include <cmath>

namespace VSTGUI {

/// @cond ignore

namespace CFontChooserInternal {

class FontPreviewView : public CView
{
public:
	FontPreviewView (const CRect& size, const CColor& color = kWhiteCColor)
	: CView (size), fontColor (color)
	{
	}
	~FontPreviewView () noexcept override {}

	void setFont (const SharedPointer<CFontDesc>& newFont)
	{
		font = newFont;
		invalid ();
	}

	void draw (CDrawContext& context) override
	{
		context.setFontColor (fontColor);
		context.setFont (font);
		std::string text;
		char string[2];
		CRect glyphRect (getViewSize ().left, getViewSize ().top, getViewSize ().left, getViewSize ().top);
		CCoord height = ceil (font->getPlatformFont ()->getAscent () + font->getPlatformFont ()->getDescent () + font->getPlatformFont ()->getLeading () + 2.);
		glyphRect.setHeight (height);
		for (int8_t i = 33; i < 126;)
		{
			while (glyphRect.right < getViewSize ().right && i < 126)
			{
				snprintf (string, 2, "%c", i++);
				text += string;
				glyphRect.setWidth (context.getStringWidth (text.c_str ()));
			}
			context.drawString (text.c_str (), glyphRect, kLeftText);
			glyphRect.left = glyphRect.right = getViewSize ().left;
			glyphRect.offset (0, height);
			text = "";
		}
	}
	
protected:
	SharedPointer<CFontDesc> font;
	CColor fontColor;
};

enum {
	kFontChooserSizeTag,
	kFontChooserBoldTag,
	kFontChooserItalicTag,
	kFontChooserUnderlineTag,
	kFontChooserStrikeoutTag
};

} // CFontChooserInternal

/// @endcond

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
CFontChooser::CFontChooser (IFontChooserDelegate* delegate,
							const SharedPointer<CFontDesc>& initialFont,
							const CFontChooserUIDefinition& uiDef)
: CViewContainer (CRect (0, 0, 300, 500)), delegate (nullptr), fontBrowser (nullptr)
{
	std::list<std::string> fnList;
	getPlatformFactory ().getAllFontFamilies ([&fnList] (const std::string& name) {
		fnList.push_back (name);
		return true;
	});
	fnList.sort ();
	std::list<std::string>::const_iterator it = fnList.begin ();
	while (it != fnList.end ())
	{
		fontNames.emplace_back (*it);
		++it;
	}
	auto dbSource = makeShared<GenericStringListDataBrowserSource> (&fontNames, this);
	dbSource->setupUI (uiDef.selectionColor, uiDef.fontColor, uiDef.rowlineColor, uiDef.rowBackColor, uiDef.rowAlternateBackColor, uiDef.font, uiDef.rowHeight);
	int32_t dbStyle = CDataBrowser::kDrawRowLines | CScrollView::kVerticalScrollbar | CScrollView::kDontDrawFrame | CScrollView::kOverlayScrollbars;
	fontBrowser = makeShared<CDataBrowser> (CRect (0, 0, 200, 500), dbSource.get (), dbStyle,
											uiDef.scrollbarWidth);
	fontBrowser->setAutosizeFlags (kAutosizeLeft | kAutosizeTop | kAutosizeBottom);
	fontBrowser->setTransparency (true);
	if (auto scrollbar = fontBrowser->getVerticalScrollbar ())
	{
		scrollbar->setBackgroundColor (uiDef.scrollbarBackgroundColor);
		scrollbar->setFrameColor (uiDef.scrollbarFrameColor);
		scrollbar->setScrollerColor (uiDef.scrollbarScrollerColor);
	}
	addSubview (fontBrowser);
	CRect controlRect (210, 0, 300, 20);
	auto label = makeShared<CTextLabel> (controlRect, "Size:");
	label->setFont (uiDef.font);
	label->setFontColor (uiDef.fontColor);
	label->sizeToFit ();
	label->setHoriAlign (kLeftText);
	label->setTransparency (true);
	label->setAutosizeFlags (kAutosizeLeft | kAutosizeTop);
	addSubview (label);
	CRect teRect = label->getViewSize ();
	teRect.left = teRect.right + 5.;
	teRect.right = controlRect.right;
	sizeEdit = makeShared<CTextEdit> (teRect, this, CFontChooserInternal::kFontChooserSizeTag);
	sizeEdit->setFont (uiDef.font);
	sizeEdit->setFontColor (uiDef.fontColor);
	sizeEdit->setHoriAlign (kLeftText);
	sizeEdit->setTransparency (true);
	sizeEdit->setAutosizeFlags (kAutosizeLeft | kAutosizeTop);
	sizeEdit->setMax (2000);
	sizeEdit->setMin (6);
	sizeEdit->setValue (2000);
	sizeEdit->sizeToFit ();
	sizeEdit->setStringToValueFunction ([] (UTF8StringPtr txt, float& result, CTextEdit* textEdit) { result = UTF8StringView (txt).toFloat (); return true; });
	addSubview (sizeEdit);
	controlRect.offset (0, 20);
	boldBox = makeShared<CCheckBox> (controlRect, this, CFontChooserInternal::kFontChooserBoldTag,
									 "Bold");
	boldBox->setFont (uiDef.font);
	boldBox->setFontColor (uiDef.fontColor);
	boldBox->setAutosizeFlags (kAutosizeLeft | kAutosizeTop);
	boldBox->sizeToFit ();
	addSubview (boldBox);
	controlRect.offset (0, 20);
	italicBox = makeShared<CCheckBox> (controlRect, this,
									   CFontChooserInternal::kFontChooserItalicTag, "Italic");
	italicBox->setFont (uiDef.font);
	italicBox->setFontColor (uiDef.fontColor);
	italicBox->setAutosizeFlags (kAutosizeLeft | kAutosizeTop);
	italicBox->sizeToFit ();
	addSubview (italicBox);
	controlRect.offset (0, 20);
	underlineBox = makeShared<CCheckBox> (
		controlRect, this, CFontChooserInternal::kFontChooserUnderlineTag, "Underline");
	underlineBox->setFont (uiDef.font);
	underlineBox->setFontColor (uiDef.fontColor);
	underlineBox->setAutosizeFlags (kAutosizeLeft | kAutosizeTop);
	underlineBox->sizeToFit ();
	addSubview (underlineBox);
	controlRect.offset (0, 20);
	strikeoutBox = makeShared<CCheckBox> (
		controlRect, this, CFontChooserInternal::kFontChooserStrikeoutTag, "Strikeout");
	strikeoutBox->setFont (uiDef.font);
	strikeoutBox->setFontColor (uiDef.fontColor);
	strikeoutBox->setAutosizeFlags (kAutosizeLeft | kAutosizeTop);
	strikeoutBox->sizeToFit ();
	addSubview (strikeoutBox);

	auto container =
		makeShared<CViewContainer> (CRect (controlRect.left, controlRect.bottom + 10, 300, 500));
	container->setBackgroundColor (uiDef.previewBackgroundColor);
	container->setAutosizeFlags (kAutosizeTop | kAutosizeBottom | kAutosizeLeft | kAutosizeRight);
	fontPreviewView = makeShared<CFontChooserInternal::FontPreviewView> (
		CRect (10, 10, container->getWidth () - 10, container->getHeight () - 10),
		uiDef.previewTextColor);
	fontPreviewView->setAutosizeFlags (kAutosizeAll);
	container->addSubview (fontPreviewView);
	addSubview (container);

	setFont (initialFont ? initialFont : kSystemFont);
	
	sizeToFit ();
	
	this->delegate = delegate;
}

//-----------------------------------------------------------------------------
CFontChooser::~CFontChooser () noexcept {}

//-----------------------------------------------------------------------------
void CFontChooser::setFont (const SharedPointer<CFontDesc>& font)
{
	if (font)
	{
		selFont = owned (new CFontDesc (*font.get ()));
		sizeEdit->setValue ((float)font->getSize ());
		boldBox->setValue ((font->getStyle () & kBoldFace) ? 1.f : 0.f);
		italicBox->setValue ((font->getStyle () & kItalicFace) ? 1.f : 0.f);
		underlineBox->setValue ((font->getStyle () & kUnderlineFace) ? 1.f : 0.f);
		strikeoutBox->setValue ((font->getStyle () & kStrikethroughFace) ? 1.f : 0.f);

		auto it = fontNames.begin ();
		int32_t row = 0;
		while (it != fontNames.end ())
		{
			if (*it == font->getName ())
			{
				fontBrowser->setSelectedRow (row, true);
				break;
			}
			++it;
			row++;
		}
		fontPreviewView.cast<CFontChooserInternal::FontPreviewView> ()->setFont (selFont);
	}
	invalid ();
}

//-----------------------------------------------------------------------------
void CFontChooser::valueChanged (CControl& pControl)
{
	if (selFont == nullptr)
		return;

	switch (pControl.getTag ())
	{
		case CFontChooserInternal::kFontChooserSizeTag:
		{
			pControl.setValue (pControl.getValue ());
			selFont->setSize (pControl.getValue ());
			break;
		}
		case CFontChooserInternal::kFontChooserBoldTag:
		{
			if (pControl.getValue () == 1)
				selFont->setStyle (selFont->getStyle () | kBoldFace);
			else
				selFont->setStyle (selFont->getStyle () & ~kBoldFace);
			break;
		}
		case CFontChooserInternal::kFontChooserItalicTag:
		{
			if (pControl.getValue () == 1)
				selFont->setStyle (selFont->getStyle () | kItalicFace);
			else
				selFont->setStyle (selFont->getStyle () & ~kItalicFace);
			break;
		}
		case CFontChooserInternal::kFontChooserUnderlineTag:
		{
			if (pControl.getValue () == 1)
				selFont->setStyle (selFont->getStyle () | kUnderlineFace);
			else
				selFont->setStyle (selFont->getStyle () & ~kUnderlineFace);
			break;
		}
		case CFontChooserInternal::kFontChooserStrikeoutTag:
		{
			if (pControl.getValue () == 1)
				selFont->setStyle (selFont->getStyle () | kStrikethroughFace);
			else
				selFont->setStyle (selFont->getStyle () & ~kStrikethroughFace);
			break;
		}
	}
	if (delegate)
		delegate->fontChanged (*this, selFont);
	fontPreviewView.cast<CFontChooserInternal::FontPreviewView> ()->setFont (selFont);
}

//-----------------------------------------------------------------------------
void CFontChooser::dbSelectionChanged (int32_t selectedRow, GenericStringListDataBrowserSource* source)
{
	if (selectedRow >= 0 && static_cast<size_t> (selectedRow) <= fontNames.size ())
		selFont->setName (fontNames[static_cast<size_t> (selectedRow)].data ());
	fontPreviewView.cast<CFontChooserInternal::FontPreviewView> ()->setFont (selFont);
	if (delegate)
		delegate->fontChanged (*this, selFont);
}

//-----------------------------------------------------------------------------
bool CFontChooser::attached (const SharedPointer<CViewContainer>& parent)
{
	if (CViewContainer::attached (parent))
	{
		fontBrowser->makeRowVisible (fontBrowser->getSelectedRow ());
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
void CFontChooser::onKeyboardEvent (KeyboardEvent& event)
{
	fontBrowser->onKeyboardEvent (event);
}

} // VSTGUI
