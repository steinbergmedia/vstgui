// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "genericoptionmenu.h"

#include "../../cdatabrowser.h"
#include "../../cfont.h"
#include "../../cframe.h"
#include "../../clayeredviewcontainer.h"
#include "../../coffscreencontext.h"
#include "../../controls/coptionmenu.h"
#include "../../cgraphicspath.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace GenericOptionMenuDetail {

static constexpr auto CheckMarkString = "\xe2\x9c\x93";

using ClickCallback = std::function<void(COptionMenu* menu, int32_t itemIndex)>;

//------------------------------------------------------------------------
class DataSource
	: public IDataBrowserDelegate
	, public IViewMouseListenerAdapter
	, public NonAtomicReferenceCounted
{
public:
	DataSource (COptionMenu* menu, const ClickCallback& clickCallback, GenericOptionMenuTheme theme)
		: menu (menu), clickCallback (clickCallback), theme (theme)
	{
		vstgui_assert (menu->getNbEntries () > 0);
	}

	void dbAttached (CDataBrowser* browser) override
	{
		db = browser;
		db->registerViewMouseListener (this);
	}

	void dbRemoved (CDataBrowser* browser) override
	{
		vstgui_assert (db == browser, "unexpected");
		db->unregisterViewMouseListener (this);
		db = nullptr;
		clickCallback (menu, CDataBrowser::kNoSelection);
	}

	void viewOnMouseExited (CView* view) override
	{
		vstgui_assert (db, "unexpected");
		db->setSelectedRow (CDataBrowser::kNoSelection);
	}

	int32_t dbGetNumRows (CDataBrowser* browser) override { return menu->getNbEntries (); }
	int32_t dbGetNumColumns (CDataBrowser* browser) override { return 1; }
	CCoord dbGetCurrentColumnWidth (int32_t index, CDataBrowser* browser) override
	{
		return browser->getWidth ();
	}
	CCoord dbGetRowHeight (CDataBrowser* browser) override
	{
		return std::ceil (theme.font->getSize () + 8);
	}
	void dbDrawHeader (CDrawContext*, const CRect&, int32_t, int32_t, CDataBrowser*) override {}

	void alterSelection (int32_t index, int32_t direction)
	{
		if (index == CDataBrowser::kNoSelection)
		{
			if (direction == 1)
				index = -1;
			else
				index = menu->getNbEntries ();
		}
		index += direction;
		if (auto item = menu->getEntry (index))
		{
			if (item->isEnabled () && !item->isSeparator ())
				db->setSelectedRow (index);
			else
				alterSelection (index, direction);
		}
	}

	int32_t dbOnKeyDown (const VstKeyCode& key, CDataBrowser* browser) override
	{
		if (key.character == 0 && key.modifier == 0)
		{
			switch (key.virt)
			{
				case VKEY_DOWN:
				{
					alterSelection (browser->getSelectedRow (), 1);
					return 1;
				}
				case VKEY_UP:
				{
					alterSelection (browser->getSelectedRow (), -1);
					return 1;
				}
				case VKEY_ESCAPE:
				{
					clickCallback (menu, CDataBrowser::kNoSelection);
					return 1;
				}
				default:
					break;
			}
		}
		return -1;
	}

	CMouseEventResult dbOnMouseMoved (const CPoint& where,
									  const CButtonState& buttons,
									  int32_t row,
									  int32_t column,
									  CDataBrowser* browser) override
	{
		if (auto item = menu->getEntry (row))
		{
			if (item->isSeparator () || !item->isEnabled ())
				browser->setSelectedRow (CDataBrowser::kNoSelection);
			else if (browser->getSelectedRow () != row)
				browser->setSelectedRow (row);
		}
		return kMouseEventHandled;
	}

	CMouseEventResult dbOnMouseDown (const CPoint& where,
									 const CButtonState& buttons,
									 int32_t row,
									 int32_t column,
									 CDataBrowser* browser) override
	{
		return kMouseEventHandled;
	}

	CMouseEventResult dbOnMouseUp (const CPoint& where,
								   const CButtonState& buttons,
								   int32_t row,
								   int32_t column,
								   CDataBrowser* browser) override
	{
		if (auto item = menu->getEntry (row))
		{
			if (!item->isSeparator () && item->isEnabled () && clickCallback)
				clickCallback (menu, row);
		}
		return kMouseEventHandled;
	}

	void drawCheckMark (CDrawContext* context, CRect size, bool selected)
	{
		if (auto checkMarkPath = owned (context->createGraphicsPath ()))
		{
			size.inset (3, 5);
			checkMarkPath->beginSubpath ({size.left, size.top + size.getHeight () / 2.});
			checkMarkPath->addLine ({size.left + size.getWidth () / 3., size.bottom});
			checkMarkPath->addLine ({size.right, size.top});
			context->setFrameColor (selected ? theme.selectedTextColor : theme.textColor);
			context->drawGraphicsPath (checkMarkPath, CDrawContext::kPathStroked);
		}
	}

	void dbDrawCell (CDrawContext* context,
					 const CRect& size,
					 int32_t row,
					 int32_t column,
					 int32_t flags,
					 CDataBrowser* browser) override
	{
		if (auto item = menu->getEntry (row))
		{
			if (item->isSeparator ())
			{
				context->setFillColor (theme.backgroundColor);
				context->drawRect (size, kDrawFilled);
				context->setFillColor (theme.textColor);
				auto r = size;
				r.inset (0, r.getHeight () / 2);
				r.setHeight (1.);
				context->drawRect (r, kDrawFilled);
				return;
			}
			context->saveGlobalState ();
			context->setDrawMode (kAntiAliasing);
			if (flags & kRowSelected)
			{
				context->setFillColor (theme.selectedBackgroundColor);
				context->drawRect (size, kDrawFilled);
				context->setFontColor (theme.selectedTextColor);
			}
			else
			{
				context->setFontColor (item->isEnabled () ? theme.textColor
														  : theme.disabledTextColor);
			}
			context->setFont (theme.font);
			if (item->isChecked ())
			{
				auto r = size;
				r.setWidth (getCheckmarkSize (context));
				drawCheckMark (context, r, flags & kRowSelected);
			}
			auto r = size;
			r.inset (getCheckmarkSize (context), 0);
			context->drawString (item->getTitle ().getPlatformString (), r, kLeftText);
			context->restoreGlobalState ();
		}
	}

	CCoord calculateMaxWidth (CFrame* frame)
	{
		auto context = COffscreenContext::create (frame, 1, 1);
		CCoord maxWidth = 0.;
		for (auto& item : *menu->getItems ())
		{
			if (item->isSeparator ())
				continue;
			auto width = context->getStringWidth (item->getTitle ());
			if (maxWidth < width)
				maxWidth = width;
		}
		maxWidth += getCheckmarkSize (context) * 2.;
		return maxWidth;
	}

	CCoord calculateMaxHeight () { return menu->getNbEntries () * dbGetHeaderHeight (nullptr); }

private:
	CCoord getCheckmarkSize (CDrawContext* context)
	{
		if (checkmarkSize == 0.)
			checkmarkSize = theme.font->getSize ();
		return checkmarkSize;
	}

	COptionMenu* menu;
	CDataBrowser* db{nullptr};
	SharedPointer<CGraphicsPath> checkMarkPath;
	ClickCallback clickCallback;
	CCoord checkmarkSize{0.};
	GenericOptionMenuTheme theme;
};

//------------------------------------------------------------------------
}

//------------------------------------------------------------------------
struct GenericOptionMenu::Impl
{
	SharedPointer<CFrame> frame;
	SharedPointer<COptionMenu> menu;
	SharedPointer<CLayeredViewContainer> container;
	ModalViewSession* modalViewSession{nullptr};
	IGenericOptionMenuListener* listener{nullptr};
	GenericOptionMenuTheme theme;
	Callback callback;
	CButtonState initialButtons;
	bool focusDrawingWasEnabled{false};
};

//------------------------------------------------------------------------
GenericOptionMenu::GenericOptionMenu (CFrame* frame,
									  CButtonState initialButtons,
									  GenericOptionMenuTheme theme)
{
	impl = std::unique_ptr<Impl> (new Impl);
	impl->frame = frame;
	impl->initialButtons = initialButtons;
	impl->theme = theme;
	impl->container = new CLayeredViewContainer (impl->frame->getViewSize ());
	impl->container->setZIndex (100);
	impl->container->setTransparency (true);
	impl->container->registerViewMouseListener (this);
	impl->modalViewSession = impl->frame->beginModalViewSession (impl->container);
	impl->focusDrawingWasEnabled = impl->frame->focusDrawingEnabled ();
	impl->frame->setFocusDrawingEnabled (false);
}

//------------------------------------------------------------------------
GenericOptionMenu::~GenericOptionMenu () noexcept
{
	impl->frame->setFocusDrawingEnabled (impl->focusDrawingWasEnabled);
}

//------------------------------------------------------------------------
void GenericOptionMenu::setListener (IGenericOptionMenuListener* listener)
{
	impl->listener = listener;
}

//------------------------------------------------------------------------
void GenericOptionMenu::removeModalView (PlatformOptionMenuResult result)
{
	if (impl->callback)
	{
		if (impl->listener)
			impl->listener->optionMenuPopupStopped ();

		auto self = shared (this);
		auto f = [self, result]() {
			auto callback = std::move (self->impl->callback);
			self->impl->callback = nullptr;
			self->impl->container->unregisterViewMouseListener (self);
			self->impl->frame->endModalViewSession (self->impl->modalViewSession);
			callback (self->impl->menu, result);
			self->impl->container = nullptr;
		};

		impl->frame->doAfterEventProcessing (f);
	}
}

//------------------------------------------------------------------------
CMouseEventResult GenericOptionMenu::viewOnMouseDown (CView* view, CPoint pos, CButtonState buttons)
{
	if (auto container = view->asViewContainer ())
	{
		CViewContainer::ViewList views;
		if (container->getViewsAt (pos, views))
		{
			return kMouseEventNotHandled;
		}
		auto self = shared (this);
		self->removeModalView ({nullptr, -1});
		return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
	}
	return kMouseEventNotHandled;
}

//------------------------------------------------------------------------
void GenericOptionMenu::popup (COptionMenu* optionMenu, const Callback& callback)
{
	impl->menu = optionMenu;
	impl->callback = callback;

	auto self = shared (this);
	auto clickCallback = [self](COptionMenu* menu, int32_t index) {
		self->removeModalView ({menu, index});
	};

	auto dataSource =
		makeOwned<GenericOptionMenuDetail::DataSource> (optionMenu, clickCallback, impl->theme);
	auto maxWidth = dataSource->calculateMaxWidth (impl->frame);
	auto viewRect = optionMenu->translateToGlobal (optionMenu->getViewSize ());
	auto where = viewRect.getCenter ();
	if (optionMenu->isPopupStyle ())
	{
		auto offset = optionMenu->getValue () * dataSource->dbGetRowHeight (nullptr);
		viewRect.offset (0, -offset);
	}
	else
	{
		viewRect.top = viewRect.bottom;
	}
	bool multipleCheck = optionMenu->isMultipleCheckStyle ();
	if (!multipleCheck && optionMenu->isCheckStyle ())
	{
		optionMenu->checkEntryAlone (static_cast<int32_t> (optionMenu->getValue ()));
	}
	viewRect.setHeight (dataSource->calculateMaxHeight ());
	if (viewRect.getWidth () < maxWidth)
		viewRect.setWidth (maxWidth);
	if (auto fr = optionMenu->getFrame ())
	{
		auto frSize = fr->getViewSize ();
		frSize.inset (12, 12); // 12 pixel margin

		if (frSize.bottom < viewRect.bottom)
		{
			viewRect.offset (0, frSize.bottom - viewRect.bottom);
		}
		if (frSize.right < viewRect.right)
		{
			viewRect.offset (-(viewRect.right - frSize.right), 0);
		}
	}
	viewRect.makeIntegral ();
	auto browser = new CDataBrowser (
		viewRect, dataSource, CDataBrowser::kDontDrawFrame | CDataBrowser::kVerticalScrollbar, 0);
	browser->setBackgroundColor (impl->theme.backgroundColor);
	impl->container->addView (browser);

	impl->frame->setFocusView (browser);
	impl->frame->onMouseMoved (where, impl->initialButtons);
	if (impl->listener)
		impl->listener->optionMenuPopupStarted ();
}

//------------------------------------------------------------------------
} // VSTGUI
