// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../ccolor.h"
#include "../cdrawcontext.h"
#include "ccontrol.h"
#include <functional>
#include <vector>

//------------------------------------------------------------------------
namespace VSTGUI {

class CListControl; // TODO: move to fwd header

//------------------------------------------------------------------------
struct CListControlRowDesc
{
	enum Flags
	{
		Selectable = 1 << 0,
		Hoverable = 1 << 1,
	};
	CCoord height {0};
	int32_t flags {Selectable};

	CListControlRowDesc () = default;
	CListControlRowDesc (CCoord h, int32_t f) : height (h), flags (f) {}
};

//------------------------------------------------------------------------
class IListControlDrawer : virtual public IReference
{
public:
	virtual ~IListControlDrawer () noexcept {}

	enum Flags
	{
		Selectable = 1 << 0,
		Selected = 1 << 1,
		Hovered = 1 << 2,
		LastRow = 1 << 3,
	};

	virtual void drawRow (CDrawContext* context, CRect size, int32_t row, int32_t flags) = 0;
};

//------------------------------------------------------------------------
class IListControlConfigurator : virtual public IReference
{
public:
	virtual ~IListControlConfigurator () noexcept {}

	virtual CListControlRowDesc getRowDesc (int32_t row) = 0;
};

//------------------------------------------------------------------------
class CListControl : public CControl
{
public:
	CListControl (const CRect& size, IControlListener* listener, int32_t tag);

	void setDrawer (IListControlDrawer* d);
	void setConfigurator (IListControlConfigurator* c);

	IListControlDrawer* getDrawer () const { return drawer; }
	IListControlConfigurator* getConfigurator () const { return configurator; }

	void recalculateHeight ();
	void invalidRow (int32_t row);

	void draw (CDrawContext* context) override;
	void drawRect (CDrawContext* context, const CRect& updateRect) override;

	void setMin (float val) override;
	void setMax (float val) override;

	CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseUp (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseExited (CPoint& where, const CButtonState& buttons) override;

	CLASS_METHODS_NOCOPY (CListControl, CControl)
private:
	int32_t getNumRows () const;
	int32_t getIntValue () const;
	int32_t getRowAtPoint (CPoint where) const;

	SharedPointer<IListControlDrawer> drawer;
	SharedPointer<IListControlConfigurator> configurator;

	std::vector<CListControlRowDesc> rowDescriptions;
	int32_t hoveredRow {-1};
};

//------------------------------------------------------------------------
class StringListControlDrawer : public IListControlDrawer, public NonAtomicReferenceCounted
{
public:
	using Func = std::function<SharedPointer<IPlatformString> (int32_t row)>;

	template <typename F>
	StringListControlDrawer (F getStringFunc) : func (std::forward<F> (getStringFunc))
	{
	}
	StringListControlDrawer () = default;

	void setGetStringFunc (Func&& getStringFunc) { func = std::move (getStringFunc); }
	void setGetStringFunc (const Func& getStringFunc) { func = getStringFunc; }

	void setFont (CFontRef f) { font = f; }
	void setFontColor (CColor color) { fontColor = color; }
	void setSelectedFontColor (CColor color) { fontColorSelected = color; }
	void setBackColor (CColor color) { backColor = color; }
	void setSelectedBackColor (CColor color) { backColorSelected = color; }
	void setLineColor (CColor color) { lineColor = color; }
	void setLineWidth (CCoord width) { lineWidth = width; }
	void setTextInset (CCoord inset) { textInset = inset; }

	CFontRef getFont () const { return font; }
	CColor getFontColor () const { return fontColor; }
	CColor getSelectedFontColor () const { return fontColorSelected; }
	CColor getBackColor () const { return backColor; }
	CColor getSelectedBackColor () const { return backColorSelected; }
	CColor getLineColor () const { return lineColor; }
	CCoord getLineWidth () const { return lineWidth; }
	CCoord getTextInset () const { return textInset; }

	void drawRow (CDrawContext* context, CRect size, int32_t row, int32_t flags) override
	{
		context->setFillColor (flags & Selected ? backColorSelected : backColor);
		context->drawRect (size, kDrawFilled);

		auto lw = lineWidth == -1 ? context->getHairlineSize () : lineWidth;
		size.bottom -= lw;

		if (!(flags & LastRow))
		{
			context->setFrameColor (lineColor);
			context->setLineWidth (lw);
			context->drawLine (size.getBottomLeft (), size.getBottomRight ());
		}
		if (func)
		{
			if (auto string = func (row))
			{
				size.inset (textInset, 0);
				context->setFontColor (flags & Selected ? fontColorSelected : fontColor);
				context->setFont (font);
				context->drawString (string, size, kLeftText);
			}
		}
	}

private:
	Func func;

	SharedPointer<CFontDesc> font;
	CColor fontColor {kBlackCColor};
	CColor fontColorSelected {kWhiteCColor};
	CColor backColor {kWhiteCColor};
	CColor backColorSelected {kBlueCColor};
	CColor lineColor {kBlackCColor};
	CCoord lineWidth {1.};
	CCoord textInset {5.};
};

//------------------------------------------------------------------------
class ListControlDrawerFunction : public IListControlDrawer, public NonAtomicReferenceCounted
{
public:
	using Func = std::function<void (CDrawContext*, CRect, int32_t, int32_t)>;

	template <typename F>
	ListControlDrawerFunction (F drawFunc) : func (std::forward<F> (drawFunc))
	{
	}

	void drawRow (CDrawContext* context, CRect size, int32_t row, int32_t flags) override
	{
		func (context, size, row, flags);
	}

private:
	Func func;
};

//------------------------------------------------------------------------
class StaticListControlConfigurator : public IListControlConfigurator,
                                      public NonAtomicReferenceCounted
{
public:
	StaticListControlConfigurator (CCoord inRowHeight,
	                               int32_t inFlags = CListControlRowDesc::Selectable)
	: rowHeight (inRowHeight), flags (inFlags)
	{
	}

	void setRowHeight (CCoord height) { rowHeight = height; }
	CCoord getRowHeight () const { return rowHeight; }

	CListControlRowDesc getRowDesc (int32_t row) override { return {rowHeight, flags}; }

private:
	CCoord rowHeight;
	int32_t flags;
};

//------------------------------------------------------------------------
} // VSTGUI
