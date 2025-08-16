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
#include "vstgui/lib/cframe.h"
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
	ViewWithAFrame (uint32_t index = 0u) : CView ({}), index (index) {}

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
struct GridLayoutWindowController : public WindowControllerAdapter
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

	void setGridProperties (const GridLayoutProperties& properties)
	{
		layouter->setProperties (properties);
		if (container)
		{
			container->invalid ();
			container->removeAll ();
			size_t numChilds = 0;
			if (properties.gridAreas.empty ())
				numChilds = properties.rows * properties.columns;
			else
				numChilds = properties.gridAreas.size ();
			for (auto i = 0u; i < numChilds; ++i)
			{
				container->addView (new ViewWithAFrame (i));
			}
			if (auto layout = container->calculateViewLayout (container->getViewSize ()))
			{
				container->applyViewLayout (*layout);
			}
		}
	}

	SharedPointer<CFrame> frame;
	CViewContainer* container {nullptr};
	SharedPointer<GridLayouter> layouter {owned (new GridLayouter ())};
};

//------------------------------------------------------------------------
class GridLayoutTestApp : public Application::DelegateAdapter,
						  public WindowListenerAdapter
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

		values->addValue (Value::make ("Add Auto Row"),
						  UIDesc::ValueCalls::onEndEdit ([this] (auto&) {
							  autoRows.push_back ({GridLayoutProperties::Auto {}});
							  if (autoRowsController)
								  autoRowsController->onRowAdded ();
						  }));
		values->addValue (Value::make ("Remove Auto Row"),
						  UIDesc::ValueCalls::onEndEdit ([this] (auto&) {
							  if (autoRowsController)
								  autoRowsController->removeSelection ();
						  }));
		values->addValue (Value::make ("Add Auto Column"),
						  UIDesc::ValueCalls::onEndEdit ([this] (auto&) {
							  autoColumns.push_back ({GridLayoutProperties::Auto {}});
							  if (autoColumnsController)
								  autoColumnsController->onRowAdded ();
						  }));
		values->addValue (Value::make ("Remove Auto Column"),
						  UIDesc::ValueCalls::onEndEdit ([this] (auto&) {
							  if (autoColumnsController)
								  autoColumnsController->removeSelection ();
						  }));

		values->addValue (Value::make ("Add Grid Area Item"),
						  UIDesc::ValueCalls::onEndEdit ([this] (auto&) {
							  gridAreas.push_back ({});
							  if (gridAreaController)
								  gridAreaController->onRowAdded ();
						  }));
		values->addValue (Value::make ("Remove Grid Area Item"),
						  UIDesc::ValueCalls::onEndEdit ([this] (auto&) {
							  if (gridAreaController)
								  gridAreaController->removeSelection ();
						  }));

		autoRows.push_back ({GridLayoutProperties::Auto {}});
		autoRows.push_back ({GridLayoutProperties::Auto {}});
		autoRows.push_back ({GridLayoutProperties::Auto {}});

		autoColumns.push_back ({GridLayoutProperties::Auto {}});
		autoColumns.push_back ({GridLayoutProperties::Auto {}});
		autoColumns.push_back ({GridLayoutProperties::Auto {}});
	}

	void finishLaunching () override
	{
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
		}
		else
		{
			IApplication::instance ().quit ();
		}
	}
	void onClosed (const IWindow& window) override { IApplication::instance ().quit (); }

	void modelUpdated ()
	{
		if (!gridLayoutWindowController)
			return;
		GridLayoutProperties props;
		if (auto v = values->getValue ("Rows"))
		{
			props.rows = std::round (Value::currentPlainValue (*v));
		}
		if (auto v = values->getValue ("Cols"))
		{
			props.columns = std::round (Value::currentPlainValue (*v));
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
		props.autoRows = autoRows;
		props.autoColumns = autoColumns;
		props.gridAreas = gridAreas;
		gridLayoutWindowController->setGridProperties (props);
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
