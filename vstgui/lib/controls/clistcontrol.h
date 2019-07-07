// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "ccontrol.h"
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

	virtual void drawBackground (CDrawContext* context, CRect size) = 0;
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
	CListControl (const CRect& size, IControlListener* listener = nullptr, int32_t tag = -1);

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
	bool doHoverCheck {false};
};

//------------------------------------------------------------------------
class StaticListControlConfigurator : public IListControlConfigurator,
                                      public NonAtomicReferenceCounted
{
public:
	StaticListControlConfigurator (CCoord inRowHeight,
	                               int32_t inFlags = CListControlRowDesc::Selectable |
	                                                 CListControlRowDesc::Hoverable)
	: rowHeight (inRowHeight), flags (inFlags)
	{
	}

	void setRowHeight (CCoord height) { rowHeight = height; }
	void setFlags (int32_t f) { flags = f; }
	CCoord getRowHeight () const { return rowHeight; }
	int32_t getFlags () const { return flags; }

	CListControlRowDesc getRowDesc (int32_t row) override { return {rowHeight, flags}; }

private:
	CCoord rowHeight;
	int32_t flags;
};

//------------------------------------------------------------------------
} // VSTGUI
