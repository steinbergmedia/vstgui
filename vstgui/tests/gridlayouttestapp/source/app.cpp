// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "vstgui/standalone/include/iapplication.h"
#include "vstgui/standalone/include/iuidescwindow.h"
#include "vstgui/standalone/include/helpers/appdelegate.h"
#include "vstgui/standalone/include/helpers/windowcontroller.h"
#include "vstgui/standalone/include/helpers/windowlistener.h"
#include "vstgui/standalone/include/helpers/uidesc/customization.h"
#include "vstgui/standalone/include/helpers/uidesc/modelbinding.h"
#include "vstgui/standalone/include/helpers/value.h"
#include "vstgui/standalone/include/helpers/menubuilder.h"
#include "vstgui/standalone/include/helpers/preferences.h"
#include "vstgui/lib/cframe.h"
#include "vstgui/lib/cclipboard.h"
#include "vstgui/lib/cdatabrowser.h"
#include "vstgui/lib/cdrawcontext.h"
#include "vstgui/lib/controls/coptionmenu.h"
#include "vstgui/lib/controls/cbuttons.h"
#include "vstgui/lib/idatabrowserdelegate.h"
#include "vstgui/lib/viewlayouter/gridlayouter.h"
#include "vstgui/uidescription/delegationcontroller.h"
#include "vstgui/uidescription/uiattributes.h"
#include "vstgui/uidescription/iuidescription.h"

using namespace VSTGUI;
using namespace VSTGUI::Standalone;

//------------------------------------------------------------------------
struct ViewWithAFrame : CView
{
	ViewWithAFrame (uint32_t index = 0u) : CView ({0., 0., 40., 40.}), index (index) {}

	void drawRect (CDrawContext* pContext, const CRect& updateRect) override
	{
		pContext->setFrameColor (frameColor);
		pContext->setFillColor (fillColor);
		pContext->drawRect (getViewSize (), kDrawFilledAndStroked);
		pContext->setFont (kSystemFont);
		pContext->setFontColor (kWhiteCColor);
		UTF8String str = toString (index);
		pContext->drawString (str, getViewSize (), CHoriTxtAlign::kCenterText);
	}

	uint32_t index {0};
	CColor frameColor {kRedCColor};
	CColor fillColor {MakeCColor (255, 0, 0, 20)};
};

//------------------------------------------------------------------------
template<typename DataType>
struct BaseController : public DelegationController,
						public DataBrowserDelegateAdapter,
						public NonAtomicReferenceCounted
{
	using Base = BaseController<DataType>;
	using OnUpdateFunc = std::function<void ()>;

	BaseController (IController* base, DataType& data, OnUpdateFunc&& onUpdate)
	: DelegationController (base), data (data), onUpdate (std::move (onUpdate))
	{
	}

	void onRowAdded ()
	{
		if (!getDataBrowser ())
			return;
		dataChanged ();
		getDataBrowser ()->selectRow (static_cast<int32_t> (getData ().size ()));
	}

	void removeSelection ()
	{
		if (!getDataBrowser ())
			return;
		auto selectedRow = getDataBrowser ()->getSelectedRow ();
		if (selectedRow == CDataBrowser::kNoSelection)
			return;
		getData ().erase (getData ().begin () + selectedRow);
		dataChanged ();
		auto newSelectedRow = selectedRow - 1;
		if (newSelectedRow < 0)
			newSelectedRow = 0;
		getDataBrowser ()->selectRow (newSelectedRow);
	}

protected:
	virtual UTF8String getCellText (CDataBrowser::Cell cell) const = 0;

	void dataChanged ()
	{
		if (browser)
			browser->recalculateLayout (true);
		onUpdate ();
	}

	DataType getData () const { return data; }
	CDataBrowser* getDataBrowser () const { return browser; }
	CColor getTextColor () const { return textColor; }
	CColor getSelectedRowBackground () const { return selectedRowBackground; }

private:
	CView* createView (const UIAttributes& attributes, const IUIDescription* description) override
	{
		if (auto customName = attributes.getAttributeValue (IUIDescription::kCustomViewName))
		{
			if (*customName == "DataBrowser")
			{
				int32_t style = CDataBrowser::kVerticalScrollbar | CDataBrowser::kDontDrawFrame;
				browser = new CDataBrowser (CRect {}, this, style);
				return browser;
			}
		}
		return controller->createView (attributes, description);
	}

	CView* verifyView (CView* view, const UIAttributes& attributes,
					   const IUIDescription* description) override
	{
		if (view == browser)
		{
			auto style =
				browser->getStyle () | CDataBrowser::kDrawRowLines | CDataBrowser::kDrawColumnLines;
			browser->setStyle (style);
			description->getColor ("control.text", textColor);
			description->getColor ("selection.background", selectedRowBackground);
			browser->recalculateLayout ();
			if (!data.empty ())
				browser->selectRow (0);
		}
		return controller->verifyView (view, attributes, description);
	}

	int32_t dbGetNumRows (CDataBrowser*) override
	{
		return static_cast<int32_t> (getData ().size ());
	}
	CCoord dbGetRowHeight (CDataBrowser*) override { return 14; }
	CCoord dbGetCurrentColumnWidth (int32_t index, CDataBrowser* b) override
	{
		return b->getWidth () / dbGetNumColumns (b);
	}
	bool dbGetLineWidthAndColor (CCoord& width, CColor& color, CDataBrowser*) override
	{
		width = 1;
		color = kBlackCColor;
		return true;
	}

	CMouseEventResult dbOnMouseDown (const CPoint& where, const CButtonState& buttons, int32_t row,
									 int32_t column, CDataBrowser* b) override
	{
		b->selectRow (row);
		return kMouseEventHandled;
	}

	void dbDrawCell (CDrawContext* context, const CRect& size, int32_t row, int32_t column,
					 int32_t flags, CDataBrowser*) override
	{
		if (row < 0 || row >= static_cast<int32_t> (getData ().size ()))
			return;

		if (flags & kRowSelected)
		{
			context->setFillColor (getSelectedRowBackground ());
			context->drawRect (size, kDrawFilled);
		}

		UTF8String str = getCellText ({row, column});
		if (!str.empty ())
		{
			context->setFont (kSystemFont);
			context->setFontColor (getTextColor ());
			context->drawString (str, size, kCenterText);
		}
	}

	CDataBrowser* browser {nullptr};
	DataType& data;
	OnUpdateFunc onUpdate;

	CColor textColor {kWhiteCColor};
	CColor selectedRowBackground {kBlueCColor};
};

//------------------------------------------------------------------------
struct GridAreaController : public BaseController<std::vector<GridLayoutProperties::GridArea>&>
{
	GridAreaController (IController* base, std::vector<GridLayoutProperties::GridArea>& areas,
						OnUpdateFunc&& onUpdate)
	: Base (base, areas, std::move (onUpdate))
	{
	}

private:
	int32_t dbGetNumColumns (CDataBrowser*) override { return 4; }

	UTF8String getCellText (CDataBrowser::Cell cell) const override
	{
		if (cell.row < 0 || cell.row >= static_cast<int32_t> (getData ().size ()))
			return {};
		const auto& area = getData ()[cell.row];
		switch (cell.column)
		{
			case 0:
				return toString (area.row);
			case 1:
				return toString (area.column);
			case 2:
				return toString (area.rowSpan);
			case 3:
				return toString (area.colSpan);
		}
		return {};
	}

	CMouseEventResult dbOnMouseUp (const CPoint& where, const CButtonState& buttons, int32_t row,
								   int32_t column, CDataBrowser* b) override
	{
		if (buttons & kLButton)
		{
			b->beginTextEdit ({row, column}, getCellText ({row, column}));
		}
		return kMouseEventHandled;
	}
	void dbCellTextChanged (int32_t row, int32_t column, UTF8StringPtr newText,
							CDataBrowser* b) override
	{
		auto result = UTF8StringView (newText).toInteger ();
		auto& area = getData ()[row];
		switch (column)
		{
			case 0:
				area.row = result;
				break;
			case 1:
				area.column = result;
				break;
			case 2:
				area.rowSpan = result;
				break;
			case 3:
				area.colSpan = result;
				break;
		}
		b->invalidateRow (row);
		dataChanged ();
	}
};

//------------------------------------------------------------------------
struct AutoSizeController : public BaseController<std::vector<GridLayoutProperties::SizeSpec>&>
{
	using OnUpdateFunc = std::function<void ()>;
	AutoSizeController (IController* base, std::vector<GridLayoutProperties::SizeSpec>& data,
						OnUpdateFunc&& onUpdate)
	: Base (base, data, std::move (onUpdate))
	{
	}
	~AutoSizeController () noexcept override {}

private:
	int32_t dbGetNumColumns (CDataBrowser*) override { return 2; }
	UTF8String getCellText (CDataBrowser::Cell cell) const override
	{
		UTF8String str;
		if (std::holds_alternative<CCoord> (getData ()[cell.row]))
		{
			if (cell.column == 0)
			{
				str = toString (std::get<CCoord> (getData ()[cell.row]));
			}
			else
			{
				str = "Coord";
			}
		}
		else if (std::holds_alternative<GridLayoutProperties::Percentage> (getData ()[cell.row]))
		{
			if (cell.column == 0)
			{
				str = toString (
						  std::get<GridLayoutProperties::Percentage> (getData ()[cell.row]).value) +
					  "%";
			}
			else
			{
				str = "Percentage";
			}
		}
		else
		{
			str = "Auto";
		}
		return str;
	}

	CMouseEventResult dbOnMouseUp (const CPoint& where, const CButtonState& buttons, int32_t row,
								   int32_t column, CDataBrowser* b) override
	{
		if (buttons & kLButton)
		{
			if (column == 0)
			{
				if (!std::holds_alternative<GridLayoutProperties::Auto> (getData ()[row]))
					b->beginTextEdit ({row, column}, getCellText ({row, column}));
			}
			else if (column == 1)
			{
				auto menuRect = b->getCellBounds ({row, column});
				CPoint pos;
				b->localToFrame (pos);
				menuRect.offset (pos);
				auto menu = new COptionMenu;
				menu->setViewSize (menuRect);
				menu->setStyle (COptionMenu::kPopupStyle | COptionMenu::kCheckStyle |
								COptionMenu::kNoDrawStyle);
				menu->addEntry ("Coord");
				menu->addEntry ("Percentage");
				menu->addEntry ("Auto");
				if (std::holds_alternative<GridLayoutProperties::Auto> (getData ()[row]))
				{
					menu->setValue (2.f);
				}
				else if (std::holds_alternative<GridLayoutProperties::Percentage> (getData ()[row]))
				{
					menu->setValue (1.f);
				}
				else
				{
					menu->setValue (0.f);
				}
				auto frame = b->getFrame ();
				frame->addView (menu);
				menu->popup ([this, row, frame, b] (COptionMenu* menu) {
					if (menu)
					{
						switch (menu->getLastResult ())
						{
							case 0:
								getData ()[row] = CCoord (20);
								break;
							case 1:
								getData ()[row] = GridLayoutProperties::Percentage {50.0};
								break;
							case 2:
								getData ()[row] = GridLayoutProperties::Auto {};
								break;
						}
						b->invalidateRow (row);
						dataChanged ();
						frame->removeView (menu);
					}
				});
			}
		}
		return kMouseEventHandled;
	}
	void dbCellTextChanged (int32_t row, int32_t column, UTF8StringPtr newText,
							CDataBrowser* b) override
	{
		auto result = UTF8StringView (newText).toDouble ();
		if (std::holds_alternative<CCoord> (getData ()[row]))
		{
			getData ()[row] = CCoord {result};
		}
		else if (std::holds_alternative<GridLayoutProperties::Percentage> (getData ()[row]))
		{
			getData ()[row] = GridLayoutProperties::Percentage {result};
		}
		b->invalidateRow (row);
		dataChanged ();
	}
};

//------------------------------------------------------------------------
struct GridLayoutWindowController : public WindowControllerAdapter,
									public NoMenuBuilder
{
	void onClosed (const IWindow& window) override
	{
		frame = nullptr;
		container = nullptr;
	}

	void beforeShow (IWindow& window) override
	{
		CRect r;
		r.setSize (window.getSize ());
		frame = makeOwned<CFrame> (r, nullptr);
		container = new CViewContainer ({0, 0, frame->getWidth (), frame->getHeight ()});
		container->setAutosizeFlags (kAutosizeAll);
		container->setViewLayouter (layouter);
		frame->addView (container);
		window.setContentView (frame);
	}

	void setGridProperties (const GridLayoutProperties& properties, size_t numChildsToCreate)
	{
		layouter->setProperties (properties);
		if (container)
		{
			container->invalid ();
			container->removeAll ();
			for (auto i = 0u; i < numChildsToCreate; ++i)
			{
				container->addView (new ViewWithAFrame (i));
			}
			if (auto layout = container->calculateViewLayout (container->getViewSize ()))
			{
				container->applyViewLayout (*layout);
			}
			else
			{
				container->removeAll ();
			}
		}
	}

	void copyGridPropertiesToClipboard ()
	{
		const auto& props = layouter->getProperties ();
		std::stringstream string;
		string << "GridLayoutProperties props;" << std::endl;
		string << "props.rows = " << props.rows << ";" << std::endl;
		string << "props.columns = " << props.columns << ";" << std::endl;
		string << "props.rowGap = " << props.rowGap << ";" << std::endl;
		string << "props.columnGap = " << props.columnGap << ";" << std::endl;
		switch (props.alignItems)
		{
			case GridLayoutProperties::AlignItems::Start:
				string << "props.alignItems = GridLayoutProperties::AlignItems::Start;"
					   << std::endl;
				break;
			case GridLayoutProperties::AlignItems::Center:
				string << "props.alignItems = GridLayoutProperties::AlignItems::Center;"
					   << std::endl;
				break;
			case GridLayoutProperties::AlignItems::End:
				string << "props.alignItems = GridLayoutProperties::AlignItems::End;" << std::endl;
				break;
			case GridLayoutProperties::AlignItems::Stretch:
				string << "props.alignItems = GridLayoutProperties::AlignItems::Stretch;"
					   << std::endl;
				break;
		}
		switch (props.justifyItems)
		{
			case GridLayoutProperties::JustifyItems::Start:
				string << "props.justifyItems = GridLayoutProperties::JustifyItems::Start;"
					   << std::endl;
				break;
			case GridLayoutProperties::JustifyItems::Center:
				string << "props.justifyItems = GridLayoutProperties::JustifyItems::Center;"
					   << std::endl;
				break;
			case GridLayoutProperties::JustifyItems::End:
				string << "props.justifyItems = GridLayoutProperties::JustifyItems::End;"
					   << std::endl;
				break;
			case GridLayoutProperties::JustifyItems::Stretch:
				string << "props.justifyItems = GridLayoutProperties::JustifyItems::Stretch;"
					   << std::endl;
				break;
		}
		switch (props.alignContent)
		{
			case GridLayoutProperties::AlignContent::Start:
				string << "props.alignContent = GridLayoutProperties::AlignContent::Start;"
					   << std::endl;
				break;
			case GridLayoutProperties::AlignContent::Center:
				string << "props.alignContent = GridLayoutProperties::AlignContent::Center;"
					   << std::endl;
				break;
			case GridLayoutProperties::AlignContent::End:
				string << "props.alignContent = GridLayoutProperties::AlignContent::End;"
					   << std::endl;
				break;
			case GridLayoutProperties::AlignContent::Stretch:
				string << "props.alignContent = GridLayoutProperties::AlignContent::Stretch;"
					   << std::endl;
				break;
			case GridLayoutProperties::AlignContent::SpaceBetween:
				string << "props.alignContent = GridLayoutProperties::AlignContent::SpaceBetween;"
					   << std::endl;
				break;
			case GridLayoutProperties::AlignContent::SpaceAround:
				string << "props.alignContent = GridLayoutProperties::AlignContent::SpaceAround;"
					   << std::endl;
				break;
		}
		switch (props.justifyContent)
		{
			case GridLayoutProperties::JustifyContent::Start:
				string << "props.justifyContent = GridLayoutProperties::JustifyContent::Start;"
					   << std::endl;
				break;
			case GridLayoutProperties::JustifyContent::Center:
				string << "props.justifyContent = GridLayoutProperties::JustifyContent::Center;"
					   << std::endl;
				break;
			case GridLayoutProperties::JustifyContent::End:
				string << "props.justifyContent = GridLayoutProperties::JustifyContent::End;"
					   << std::endl;
				break;
			case GridLayoutProperties::JustifyContent::Stretch:
				string << "props.justifyContent = GridLayoutProperties::JustifyContent::Stretch;"
					   << std::endl;
				break;
			case GridLayoutProperties::JustifyContent::SpaceBetween:
				string
					<< "props.justifyContent = GridLayoutProperties::JustifyContent::SpaceBetween;"
					<< std::endl;
				break;
			case GridLayoutProperties::JustifyContent::SpaceAround:
				string
					<< "props.justifyContent = GridLayoutProperties::JustifyContent::SpaceAround;"
					<< std::endl;
				break;
		}
		string << "props.autoRows.reserve (" << props.autoRows.size () << ");" << std::endl;
		for (const auto& s : props.autoRows)
		{
			if (std::holds_alternative<CCoord> (s))
			{
				string << "props.autoRows.push_back (CCoord {" << std::get<CCoord> (s) << "});"
					   << std::endl;
			}
			else if (std::holds_alternative<GridLayoutProperties::Percentage> (s))
			{
				string << "props.autoRows.push_back (GridLayoutProperties::Percentage {"
					   << std::get<GridLayoutProperties::Percentage> (s).value << "});"
					   << std::endl;
			}
			else
			{
				string << "props.autoRows.push_back (GridLayoutProperties::Auto {});" << std::endl;
			}
		}
		string << "props.autoColumns.reserve (" << props.autoColumns.size () << ");" << std::endl;
		for (const auto& s : props.autoColumns)
		{
			if (std::holds_alternative<CCoord> (s))
			{
				string << "props.autoColumns.push_back (CCoord {" << std::get<CCoord> (s) << "});"
					   << std::endl;
			}
			else if (std::holds_alternative<GridLayoutProperties::Percentage> (s))
			{
				string << "props.autoColumns.push_back (GridLayoutProperties::Percentage {"
					   << std::get<GridLayoutProperties::Percentage> (s).value << "});"
					   << std::endl;
			}
			else
			{
				string << "props.autoColumns.push_back (GridLayoutProperties::Auto {});"
					   << std::endl;
			}
		}
		string << "props.gridAreas.reserve (" << props.gridAreas.size () << ");" << std::endl;
		for (const auto& a : props.gridAreas)
		{
			string << "props.gridAreas.push_back ({" << a.row << ", " << a.column << ", "
				   << a.rowSpan << ", " << a.colSpan << "});" << std::endl;
		}

		CClipboard::setString (string.str ().data ());
	}

	SharedPointer<CFrame> frame;
	CViewContainer* container {nullptr};
	SharedPointer<GridLayouter> layouter {owned (new GridLayouter ())};
};

//------------------------------------------------------------------------
struct GridLayoutPropertiesWindowController : DelegationController
{
	GridLayoutPropertiesWindowController (IController* baseController)
	: DelegationController (baseController)
	{
	}

	CView* verifyView (CView* view, const UIAttributes& attributes,
					   const IUIDescription* description) override
	{
		if (!container)
		{
			if ((container = view->asViewContainer ()))
			{
				GridLayoutProperties grid;
				grid.rows = 15;
				grid.columns = 2;
				grid.alignItems = GridLayoutProperties::AlignItems::Stretch;
				grid.justifyItems = GridLayoutProperties::JustifyItems::Stretch;
				grid.alignContent = GridLayoutProperties::AlignContent::SpaceAround;
				grid.justifyContent = GridLayoutProperties::JustifyContent::SpaceAround;
				grid.autoRows = {
					CCoord {20.}, CCoord {20.},
					CCoord {20.}, CCoord {20.},
					CCoord {20.}, CCoord {20.},
					CCoord {20.}, CCoord {20.},
					CCoord {20.}, GridLayoutProperties::Auto {},
					CCoord {20.}, GridLayoutProperties::Auto {},
					CCoord {20.}, GridLayoutProperties::Auto {},
					CCoord {20.}, CCoord {120.},
				};
				grid.autoColumns = {
					GridLayoutProperties::Auto {},
					GridLayoutProperties::Auto {},
					CCoord {30.},
				};
				grid.gridAreas = {
					{0, 0, 1, 1},  {0, 1, 1, 1},  {1, 0, 1, 1},	 {1, 1, 1, 1},	{2, 0, 1, 1},
					{2, 1, 1, 1},  {3, 0, 1, 1},  {3, 1, 1, 1},	 {4, 0, 1, 1},	{4, 1, 1, 1},
					{5, 0, 1, 1},  {5, 1, 1, 1},  {6, 0, 1, 1},	 {6, 1, 1, 1},	{7, 0, 1, 1},
					{7, 1, 1, 1},  {8, 0, 1, 3},  {9, 0, 1, 2},	 {10, 0, 1, 2}, {11, 0, 1, 2},
					{12, 0, 1, 2}, {13, 0, 1, 2}, {14, 0, 1, 1}, {14, 1, 1, 1},
				};
				container->setViewLayouter (makeOwned<GridLayouter> (grid));
			}
		}
		return view;
	}

	CViewContainer* container {nullptr};
};

//------------------------------------------------------------------------
class GridLayoutTestApp : public Application::DelegateAdapter,
						  public WindowListenerAdapter,
						  public ICommandHandler
{
public:
	static constexpr int32_t MaxRowsCols = 50;
	static constexpr int32_t MaxGap = 2000;

	GridLayoutTestApp ()
	: Application::DelegateAdapter ({"simple_standalone", "1.0.0", VSTGUI_STANDALONE_APP_URI})
	{
		auto modelUpdatedCallback =
			UIDesc::ValueCalls::onEndEdit ([this] (auto&) { modelUpdated (); });
		values->addValue (
			Value::make ("Rows", 2. / MaxRowsCols, Value::makeRangeConverter (1, MaxRowsCols, 0)),
			modelUpdatedCallback);
		values->addValue (
			Value::make ("Cols", 2. / MaxRowsCols, Value::makeRangeConverter (1, MaxRowsCols, 0)),
			modelUpdatedCallback);
		values->addValue (Value::make ("Row Gap", 0., Value::makeRangeConverter (0, MaxGap, 2)),
						  modelUpdatedCallback);
		values->addValue (Value::make ("Col Gap", 0., Value::makeRangeConverter (0, MaxGap, 2)),
						  modelUpdatedCallback);
		values->addValue (
			Value::makeStringListValue ("Align Items", {"Start", "Center", "End", "Stretch"}),
			modelUpdatedCallback);
		values->addValue (
			Value::makeStringListValue ("Justify Items", {"Start", "Center", "End", "Stretch"}),
			modelUpdatedCallback);
		values->addValue (
			Value::makeStringListValue ("Align Content", {"Start", "Center", "End", "Stretch",
														  "SpaceBetween", "SpaceAround"}),
			modelUpdatedCallback);
		values->addValue (
			Value::makeStringListValue ("Justify Content", {"Start", "Center", "End", "Stretch",
															"SpaceBetween", "SpaceAround"}),
			modelUpdatedCallback);
		values->addValue (Value::makeStepValue ("NumViews", 1000), modelUpdatedCallback);

		values->addValue (Value::make ("Add Auto Row"),
						  UIDesc::ValueCalls::onEndEdit ([this] (auto& v) {
							  if (v.getValue () == 0)
								  return;
							  autoRows.push_back ({GridLayoutProperties::Auto {}});
							  if (autoRowsController)
								  autoRowsController->onRowAdded ();
							  v.performEdit (0.);
						  }));
		values->addValue (Value::make ("Remove Auto Row"),
						  UIDesc::ValueCalls::onEndEdit ([this] (auto& v) {
							  if (v.getValue () == 0)
								  return;
							  if (autoRowsController)
								  autoRowsController->removeSelection ();
							  v.performEdit (0.);
						  }));
		values->addValue (Value::make ("Add Auto Column"),
						  UIDesc::ValueCalls::onEndEdit ([this] (auto& v) {
							  if (v.getValue () == 0)
								  return;
							  autoColumns.push_back ({GridLayoutProperties::Auto {}});
							  if (autoColumnsController)
								  autoColumnsController->onRowAdded ();
							  v.performEdit (0.);
						  }));
		values->addValue (Value::make ("Remove Auto Column"),
						  UIDesc::ValueCalls::onEndEdit ([this] (auto& v) {
							  if (v.getValue () == 0)
								  return;
							  if (autoColumnsController)
								  autoColumnsController->removeSelection ();
							  v.performEdit (0.);
						  }));

		values->addValue (Value::make ("Add Grid Area Item"),
						  UIDesc::ValueCalls::onEndEdit ([this] (auto& v) {
							  if (v.getValue () == 0)
								  return;
							  gridAreas.push_back ({});
							  if (gridAreaController)
								  gridAreaController->onRowAdded ();
							  v.performEdit (0.);
						  }));
		values->addValue (Value::make ("Remove Grid Area Item"),
						  UIDesc::ValueCalls::onEndEdit ([this] (auto& v) {
							  if (v.getValue () == 0)
								  return;
							  if (gridAreaController)
								  gridAreaController->removeSelection ();
							  v.performEdit (0.);
						  }));
	}

	void finishLaunching () override
	{
		restoreValues ();
		restoreAutoRowsCols ("AutoRows", autoRows);
		restoreAutoRowsCols ("AutoColumns", autoColumns);
		restoreGridAreas (gridAreas);

		auto customization = UIDesc::Customization::make ();
		customization->addCreateViewControllerFunc (
			"AutoRowsController",
			[this] (const UTF8StringView& name, IController* parent, const IUIDescription* uiDesc) {
				autoRowsController =
					new AutoSizeController (parent, autoRows, [this] () { modelUpdated (); });
				return autoRowsController;
			});
		customization->addCreateViewControllerFunc (
			"AutoColumnsController",
			[this] (const UTF8StringView& name, IController* parent, const IUIDescription* uiDesc) {
				autoColumnsController =
					new AutoSizeController (parent, autoColumns, [this] () { modelUpdated (); });
				return autoColumnsController;
			});
		customization->addCreateViewControllerFunc (
			"GridAreaController",
			[this] (const UTF8StringView& name, IController* parent, const IUIDescription* uiDesc) {
				gridAreaController =
					new GridAreaController (parent, gridAreas, [this] () { modelUpdated (); });
				return gridAreaController;
			});
		customization->addCreateViewControllerFunc (
			"GridLayoutPropertiesWindowController",
			[this] (const UTF8StringView& name, IController* parent, const IUIDescription* uiDesc) {
				return new GridLayoutPropertiesWindowController (parent);
			});

		UIDesc::Config config;
		config.uiDescFileName = "Window.uidesc";
		config.viewName = "Window";
		config.windowConfig.title = "Grid Layout Properties";
		config.windowConfig.autoSaveFrameName = "GridLayoutProperties";
		config.customization = customization;
		config.modelBinding = values;
		config.windowConfig.style.border ().close ().centered ();
		if (auto window = UIDesc::makeWindow (config))
		{
			window->show ();
			window->registerWindowListener (this);

			gridLayoutWindowController = std::make_shared<GridLayoutWindowController> ();
			WindowConfiguration windowConfig;
			windowConfig.style.border ().size ().centered ();
			windowConfig.title = "Grid Layout Example";
			windowConfig.autoSaveFrameName = "GridLayoutExample";
			windowConfig.size = {500, 500};
			if (auto gridLayoutWindow = IApplication::instance ().createWindow (
					windowConfig, gridLayoutWindowController))
				gridLayoutWindow->show ();
			modelUpdated ();
			window->activate ();
		}
		else
		{
			IApplication::instance ().quit ();
		}
	}
	void onClosed (const IWindow& window) override { IApplication::instance ().quit (); }
	bool canHandleCommand (const Command& command) override
	{
		if (command == Commands::Copy)
			return true;
		return false;
	}
	bool handleCommand (const Command& command) override
	{
		if (command == Commands::Copy)
		{
			gridLayoutWindowController->copyGridPropertiesToClipboard ();
			return true;
		}
		return false;
	}

	void restoreValues ()
	{
		Preferences prefs ({"Values"});
		for (auto& value : values->getValues ())
		{
			const auto& id = value->getID ();
			if (auto norm = prefs.getNumber<double> (id))
			{
				value->performEdit (*norm);
			}
		}
	}

	void storeValue ()
	{
		Preferences prefs ({"Values"});
		for (auto& value : values->getValues ())
		{
			const auto& id = value->getID ();
			const auto& norm = value->getValue ();
			prefs.setFloat (id, norm);
		}
	}

	void restoreAutoRowsCols (const UTF8String& name,
							  std::vector<GridLayoutProperties::SizeSpec>& list) const
	{
		Preferences prefs ({name});
		size_t index = 0u;
		while (true)
		{
			auto sizePrefs = prefs.subGroupPreferences (toString (index));
			if (auto type = sizePrefs.get ("Type"))
			{
				if (*type == "")
					break;
				if (*type == "Coord")
				{
					CCoord coord = 20.;
					if (auto value = sizePrefs.getNumber<double> ("Value"))
						coord = *value;
					list.push_back (coord);
				}
				else if (*type == "Percentage")
				{
					GridLayoutProperties::Percentage perc {50.};
					if (auto value = sizePrefs.getNumber<double> ("Value"))
						perc.value = *value;
					list.push_back (perc);
				}
				else if (*type == "Auto")
				{
					list.push_back (GridLayoutProperties::Auto {});
				}
				else
				{
					assert (false); // unexpected
					break;
				}
			}
			else
			{
				break;
			}
			++index;
		}
	}

	void storeAutoRowsCols (const UTF8String& name,
							const std::vector<GridLayoutProperties::SizeSpec>& list) const
	{
		Preferences prefs ({name});
		size_t index = 0u;
		for (const auto& size : list)
		{
			auto sizePrefs = prefs.subGroupPreferences (toString (index));
			if (std::holds_alternative<CCoord> (size))
			{
				sizePrefs.set ("Type", "Coord");
				sizePrefs.setFloat ("Value", std::get<CCoord> (size));
			}
			else if (std::holds_alternative<GridLayoutProperties::Percentage> (size))
			{
				sizePrefs.set ("Type", "Percentage");
				sizePrefs.setFloat ("Value",
									std::get<GridLayoutProperties::Percentage> (size).value);
			}
			else
			{
				sizePrefs.set ("Type", "Auto");
			}
			++index;
		}
		auto sizePrefs = prefs.subGroupPreferences (toString (index));
		sizePrefs.set ("Type", "");
	}

	void restoreGridAreas (std::vector<GridLayoutProperties::GridArea>& areas)
	{
		Preferences prefs ({"GridAreas"});
		size_t index = 0u;
		while (true)
		{
			GridLayoutProperties::GridArea area {};
			auto areaPrefs = prefs.subGroupPreferences (toString (index));
			if (auto value = areaPrefs.getNumber<size_t> ("Row"))
				area.row = *value;
			else
				break;
			if (auto value = areaPrefs.getNumber<size_t> ("Column"))
				area.column = *value;
			else
				break;
			if (auto value = areaPrefs.getNumber<size_t> ("RowSpan"))
				area.rowSpan = *value;
			else
				break;
			if (auto value = areaPrefs.getNumber<size_t> ("ColSpan"))
				area.colSpan = *value;
			else
				break;
			areas.push_back (area);
			++index;
		}
	}

	void storeGridAreas (const std::vector<GridLayoutProperties::GridArea>& areas)
	{
		Preferences prefs ({"GridAreas"});
		size_t index = 0u;
		for (const auto& area : areas)
		{
			auto areaPrefs = prefs.subGroupPreferences (toString (index));
			areaPrefs.setNumber ("Row", area.row);
			areaPrefs.setNumber ("Column", area.column);
			areaPrefs.setNumber ("RowSpan", area.rowSpan);
			areaPrefs.setNumber ("ColSpan", area.colSpan);
			++index;
		}
		auto areaPrefs = prefs.subGroupPreferences (toString (index));
		areaPrefs.set ("Row", "");
	}

	void onQuit () override
	{
		storeValue ();
		storeAutoRowsCols ("AutoRows", autoRows);
		storeAutoRowsCols ("AutoColumns", autoColumns);
		storeGridAreas (gridAreas);
	}

	void modelUpdated ()
	{
		if (!gridLayoutWindowController)
			return;
		GridLayoutProperties props;
		if (auto v = values->getValue ("Rows"))
		{
			props.rows = static_cast<size_t> (std::round (Value::currentPlainValue (*v)));
		}
		if (auto v = values->getValue ("Cols"))
		{
			props.columns = static_cast<size_t> (std::round (Value::currentPlainValue (*v)));
		}
		if (auto v = values->getValue ("Row Gap"))
		{
			props.rowGap = Value::currentPlainValue (*v);
		}
		if (auto v = values->getValue ("Col Gap"))
		{
			props.columnGap = Value::currentPlainValue (*v);
		}
		if (auto v = values->getValue ("Align Items"))
		{
			props.alignItems =
				static_cast<GridLayoutProperties::AlignItems> (Value::currentPlainValue (*v));
		}
		if (auto v = values->getValue ("Justify Items"))
		{
			props.justifyItems =
				static_cast<GridLayoutProperties::JustifyItems> (Value::currentPlainValue (*v));
		}
		if (auto v = values->getValue ("Align Content"))
		{
			props.alignContent =
				static_cast<GridLayoutProperties::AlignContent> (Value::currentPlainValue (*v));
		}
		if (auto v = values->getValue ("Justify Content"))
		{
			props.justifyContent =
				static_cast<GridLayoutProperties::JustifyContent> (Value::currentPlainValue (*v));
		}
		size_t numViewsToCreate = 1u;
		if (auto v = values->getValue ("NumViews"))
		{
			numViewsToCreate = Value::currentPlainValue (*v);
		}

		props.autoRows = autoRows;
		props.autoColumns = autoColumns;
		props.gridAreas = gridAreas;
		gridLayoutWindowController->setGridProperties (props, numViewsToCreate);
	}

	std::shared_ptr<GridLayoutWindowController> gridLayoutWindowController;
	UIDesc::ModelBindingCallbacksPtr values {UIDesc::ModelBindingCallbacks::make ()};
	SharedPointer<AutoSizeController> autoRowsController;
	SharedPointer<AutoSizeController> autoColumnsController;
	SharedPointer<GridAreaController> gridAreaController;
	std::vector<GridLayoutProperties::SizeSpec> autoRows;
	std::vector<GridLayoutProperties::SizeSpec> autoColumns;
	std::vector<GridLayoutProperties::GridArea> gridAreas;
};

//------------------------------------------------------------------------
static Application::Init gAppDelegate (std::make_unique<GridLayoutTestApp> ());
