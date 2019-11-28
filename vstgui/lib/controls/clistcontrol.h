// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../optional.h"
#include "ccontrol.h"

//------------------------------------------------------------------------
namespace VSTGUI {

//------------------------------------------------------------------------
/**	Control which draws a list of configurable rows
 *
 *	This control needs to be setup with an instance of a IListControlDrawer and a
 *	IListControlConfigurator. The number of rows is configured via the min and max values. And the
 *	selected row is the value of this control.
 *	The actual drawing is done via the IListControlDrawer instance. And the row configuration is
 *	handled via the IListControlConfigurator instance. Every row can have different heights and
 *	flags.
 *
 *	@ingroup new_in_4_9
 */
//------------------------------------------------------------------------
class CListControl final : public CControl
{
public:
	CListControl (const CRect& size, IControlListener* listener = nullptr, int32_t tag = -1);
	~CListControl () override;

	void setDrawer (IListControlDrawer* d);
	void setConfigurator (IListControlConfigurator* c);

	IListControlDrawer* getDrawer () const;
	IListControlConfigurator* getConfigurator () const;

	void recalculateLayout ();

	void invalidRow (int32_t row);
	Optional<int32_t> getRowAtPoint (CPoint where) const;
	Optional<CRect> getRowRect (int32_t row) const;
	Optional<int32_t> getHoveredRow () const;

	int32_t getIntValue () const;
	int32_t getNumRows () const;

	// overrides
	void setMin (float val) override;
	void setMax (float val) override;

	bool attached (CView* parent) override;
	void draw (CDrawContext* context) override;
	void drawRect (CDrawContext* context, const CRect& updateRect) override;
	CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseUp (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseExited (CPoint& where, const CButtonState& buttons) override;
	int32_t onKeyDown (VstKeyCode& keyCode) override;
	void setViewSize (const CRect& rect, bool invalid = true) override;
	bool drawFocusOnTop () override;
	bool getFocusPath (CGraphicsPath& outPath) override;

	CLASS_METHODS_NOCOPY (CListControl, CControl)
private:
	int32_t getNextSelectableRow (int32_t r, int32_t direction) const;
	int32_t getMinRowIndex () const;
	int32_t getMaxRowIndex () const;
	size_t getNormalizedRowIndex (int32_t row) const;
	bool rowSelectable (int32_t row) const;
	void clearHoveredRow ();

	struct Impl;
	std::unique_ptr<Impl> impl;
};

//------------------------------------------------------------------------
/** The description of one row for the CListControl
 *
 *	This is returned by an instance of IListControlConfigurator for every row.
 *
 *	@ingroup new_in_4_9
 */
//------------------------------------------------------------------------
struct CListControlRowDesc
{
	enum Flags
	{
		/** Indicates that the row is selectable */
		Selectable = 1 << 0,
		/** Indicates that the row should be redrawn when the mouse hovers it */
		Hoverable = 1 << 1,
	};
	/** The height of the row */
	CCoord height {0};
	/** The flags of the row, see the Flags enum above */
	int32_t flags {Selectable};

	CListControlRowDesc () = default;
	CListControlRowDesc (CCoord h, int32_t f) : height (h), flags (f) {}
};

//------------------------------------------------------------------------
/** The list control drawer interface
 *
 *	This is used to do the actual drawing of the list control.
 *
 *	@ingroup new_in_4_9
 */
//------------------------------------------------------------------------
class IListControlDrawer : virtual public IReference
{
public:
	virtual ~IListControlDrawer () noexcept {}

	struct Row
	{
		enum
		{
			Selectable = 1 << 0,
			Selected = 1 << 1,
			Hovered = 1 << 2,
			LastRow = 1 << 3,
		};

		operator int32_t () const { return getIndex (); }
		int32_t getIndex () const { return index; }

		bool isSelectable () const { return (flags & Selectable) != 0; }
		bool isSelected () const { return (flags & Selected) != 0; }
		bool isHovered () const { return (flags & Hovered) != 0; }
		bool isLastRow () const { return (flags & LastRow) != 0; }

		Row (int32_t index, int32_t flags) : index (index), flags (flags) {}

	private:
		int32_t index;
		int32_t flags;
	};

	virtual void drawBackground (CDrawContext* context, CRect size) = 0;
	virtual void drawRow (CDrawContext* context, CRect size, Row row) = 0;
};

//------------------------------------------------------------------------
/** The list control configurator interface
 *
 *	@ingroup new_in_4_9
 */
//------------------------------------------------------------------------
class IListControlConfigurator : virtual public IReference
{
public:
	virtual ~IListControlConfigurator () noexcept {}

	virtual CListControlRowDesc getRowDesc (int32_t row) const = 0;
};

//------------------------------------------------------------------------
/** A list control configurator implementation.
 *
 *	Returns the same row description for all row indices
 *
 *	@ingroup new_in_4_9
 */
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

	CListControlRowDesc getRowDesc (int32_t row) const override { return {rowHeight, flags}; }

private:
	CCoord rowHeight;
	int32_t flags;
};

//------------------------------------------------------------------------
} // VSTGUI
