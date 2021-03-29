// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "genericoptionmenu.h"

#include "../../animation/animations.h"
#include "../../animation/timingfunctions.h"
#include "../../cdatabrowser.h"
#include "../../cfont.h"
#include "../../cframe.h"
#include "../../cgraphicspath.h"
#include "../../clayeredviewcontainer.h"
#include "../../coffscreencontext.h"
#include "../../controls/coptionmenu.h"
#include "../../controls/cscrollbar.h"
#include "../../cvstguitimer.h"
#include "../../idatabrowserdelegate.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace GenericOptionMenuDetail {

using ClickCallback = std::function<void (COptionMenu* menu, int32_t itemIndex)>;

class DataSource;

//------------------------------------------------------------------------
template <typename Proc>
CView* setupGenericOptionMenu (Proc clickCallback, CViewContainer* container,
                               COptionMenu* optionMenu, GenericOptionMenuTheme& theme,
                               CRect viewRect, DataSource* parentDataSource);

//------------------------------------------------------------------------
class DataSource : public DataBrowserDelegateAdapter,
                   public IMouseObserver,
                   public NonAtomicReferenceCounted
{
public:
	DataSource (CViewContainer* mainContainer, COptionMenu* menu,
	            const ClickCallback& clickCallback, GenericOptionMenuTheme theme,
	            DataSource* parentDataSource)
	: mainContainer (mainContainer)
	, menu (menu)
	, parentDataSource (parentDataSource)
	, clickCallback (clickCallback)
	, theme (theme)
	{
		vstgui_assert (menu->getNbEntries () > 0);
	}

	CCoord dbGetRowHeight (CDataBrowser* browser) override
	{
		return std::ceil (theme.font->getSize () + 8);
	}

	CCoord calculateMaxWidth (CFrame* frame)
	{
		if (maxWidth >= 0.)
			return maxWidth;
		auto context = COffscreenContext::create ({1., 1.});
		maxWidth = 0.;
		maxTitleWidth = 0.;
		hasRightMargin = false;
		for (auto& item : *menu->getItems ())
		{
			if (item->isSeparator ())
				continue;
			auto width = context->getStringWidth (item->getTitle ());
			hasRightMargin |= item->getSubmenu () ? true : false;
			hasRightMargin |= item->getIcon () ? true : false;
			if (maxTitleWidth < width)
				maxTitleWidth = width;
		}
		maxWidth = maxTitleWidth + getCheckmarkWidth () * 2.;
		if (hasRightMargin)
			maxWidth += getSubmenuIndicatorWidth ();
		return maxWidth;
	}

	CCoord calculateMaxHeight () { return menu->getNbEntries () * dbGetHeaderHeight (nullptr); }

	bool setMaxWidth (CCoord width)
	{
		vstgui_assert (maxWidth >= 0.);
		auto minWidth = getCheckmarkWidth () * 2.;
		if (hasRightMargin)
			minWidth += getSubmenuIndicatorWidth ();
		if (minWidth > width)
			return false;
		if (minWidth + maxTitleWidth < width)
		{
			return true;
		}
		maxWidth = width;
		maxTitleWidth = maxWidth - minWidth;
		return true;
	}

private:
	static constexpr int32_t ViewRemoved = -2;

	void dbAttached (CDataBrowser* browser) override
	{
		db = browser;
		db->getFrame ()->registerMouseObserver (this);
	}

	void dbRemoved (CDataBrowser* browser) override
	{
		vstgui_assert (db == browser, "unexpected");
		closeSubMenu (false);
		db->getFrame ()->unregisterMouseObserver (this);
		db = nullptr;
		clickCallback (menu, ViewRemoved);
	}

	void onMouseEntered (CView* view, CFrame* frame) override
	{
		if (view == subMenuView)
		{
			if (selectedRow >= 0)
				db->setSelectedRow (selectedRow);
		}
	}
	
	void onMouseExited (CView* view, CFrame* frame) override
	{
		if (view != db)
			return;
		selectedRow = db->getSelectedRow ();
		db->setSelectedRow (CDataBrowser::kNoSelection);
		db->getFrame ()->doAfterEventProcessing ([this] () {
			if (db->getSelectedRow () == CDataBrowser::kNoSelection && subMenuView)
			{
				closeSubMenu ();
			}
		});
	}

	int32_t dbGetNumRows (CDataBrowser* browser) override { return menu->getNbEntries (); }
	int32_t dbGetNumColumns (CDataBrowser* browser) override { return 1; }
	CCoord dbGetCurrentColumnWidth (int32_t index, CDataBrowser* browser) override
	{
		return browser->getWidth ();
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
			if (item->isEnabled () && !item->isSeparator () && !item->isTitle ())
			{
				closeSubMenu ();
				db->setSelectedRow (index, true);
			}
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
				case VKEY_RETURN:
				case VKEY_ENTER:
				{
					if (clickCallback)
						clickCallback (menu, browser->getSelectedRow ());
					return 1;
				}
				case VKEY_LEFT:
				{
					if (parentDataSource)
					{
						parentDataSource->closeSubMenu ();
						return 1;
					}
					break;
				}
				case VKEY_RIGHT:
				{
					auto row = db->getSelectedRow ();
					if (auto item = menu->getEntry (row))
					{
						if (auto subMenu = item->getSubmenu ())
						{
							auto r = db->getCellBounds ({row, 0});
							openSubMenu (item, r);
							return 1;
						}
					}
				}
				default: break;
			}
		}
		return -1;
	}

	CMouseEventResult dbOnMouseMoved (const CPoint& where, const CButtonState& buttons, int32_t row,
	                                  int32_t column, CDataBrowser* browser) override
	{
		if (auto item = menu->getEntry (row))
		{
			if (browser->getSelectedRow () != row)
			{
				closeSubMenu ();
				if (item->isSeparator () || !item->isEnabled () || item->isTitle ())
					browser->setSelectedRow (CDataBrowser::kNoSelection);
				else
				{
					browser->setSelectedRow (row, true);
					auto r = browser->getCellBounds ({row, column});
					openSubMenu (item, r);
				}
			}
		}
		return kMouseEventHandled;
	}

	CMouseEventResult dbOnMouseDown (const CPoint& where, const CButtonState& buttons, int32_t row,
	                                 int32_t column, CDataBrowser* browser) override
	{
		if (auto item = menu->getEntry (row))
		{
			if (item->isTitle () || !item->isEnabled () || item->isSeparator ())
				browser->setSelectedRow (CDataBrowser::kNoSelection);
		}
		return kMouseEventHandled;
	}

	CMouseEventResult dbOnMouseUp (const CPoint& where, const CButtonState& buttons, int32_t row,
	                               int32_t column, CDataBrowser* browser) override
	{
		if (auto item = menu->getEntry (row))
		{
			if (!item->isSeparator () && !item->isTitle () && item->isEnabled () && clickCallback)
				clickCallback (menu, row);
		}
		return kMouseEventHandled;
	}
	
	void closeSubMenu (bool allowAnimation = true)
	{
		using namespace Animation;
		if (subMenuView)
		{
			if (!allowAnimation)
			{
				subMenuView->getParentView ()->asViewContainer ()->removeView (subMenuView);
			}
			else
			{
				auto view = shared (subMenuView);
				subMenuView = nullptr;
				view->addAnimation (
				    "AlphaAnimation", new AlphaValueAnimation (0.f, true),
				    new CubicBezierTimingFunction (
				        CubicBezierTimingFunction::easyOut (theme.menuAnimationTime)),
				    [view] (CView*, const IdStringPtr, IAnimationTarget*) {
					    if (view->isAttached ())
						    view->getParentView ()->asViewContainer ()->removeView (view);
				    });
				if (db)
				{
					if (auto frame = db->getFrame ())
						frame->setFocusView (db);
				}
			}
		}
	}

	void openSubMenu (CMenuItem* item, CRect cellRect)
	{
		closeSubMenu ();
		if (auto subMenu = item->getSubmenu ())
		{
			auto callback = [this] (COptionMenu* m, int32_t index) {
				if (index != ViewRemoved)
					clickCallback (m, index);
			};
			db->translateToGlobal (cellRect, true);
			subMenuView =
			    setupGenericOptionMenu (callback, mainContainer, subMenu, theme, cellRect, this);
		}
	}

	void drawCheckMark (CDrawContext* context, CRect size, bool selected)
	{
		if (auto checkMarkPath = owned (context->createGraphicsPath ()))
		{
			CRect r (0., 0., size.getHeight () * 0.4, size.getHeight () * 0.4);
			r.centerInside (size);
			checkMarkPath->beginSubpath ({r.left, r.top + r.getHeight () / 2.});
			checkMarkPath->addLine ({r.left + r.getWidth () / 3., r.bottom});
			checkMarkPath->addLine ({r.right, r.top});
			context->setFrameColor (selected ? theme.selectedTextColor : theme.textColor);
			context->drawGraphicsPath (checkMarkPath, CDrawContext::kPathStroked);
		}
	}

	void drawSubmenuIndicator (CDrawContext* context, CRect size, bool selected)
	{
		if (auto path = owned (context->createGraphicsPath ()))
		{
			CRect r = size;
			r.setWidth (r.getWidth () / 2.);
			r.setHeight (r.getHeight () / 2.);
			r.offset (size.getHeight () / 2., size.getHeight () / 4.);
			path->beginSubpath (r.getTopLeft ());
			path->addLine (r.getBottomLeft ());
			path->addLine ({r.right, r.top + r.getHeight () / 2.});
			path->closeSubpath ();
			context->setFillColor (selected ? theme.selectedTextColor : theme.textColor);
			context->drawGraphicsPath (path, CDrawContext::kPathFilled);
		}
	}

	void drawItemIcon (CDrawContext* context, CRect size, CBitmap* bitmap)
	{
		ConcatClip cc (*context, size);
		CRect iconRect;
		iconRect.setSize (bitmap->getSize ());
		iconRect.centerInside (size);
		bitmap->draw (context, iconRect);
	}

	void dbDrawCell (CDrawContext* context, const CRect& size, int32_t row, int32_t column,
	                 int32_t flags, CDataBrowser* browser) override
	{
		if (auto item = menu->getEntry (row))
		{
			context->setDrawMode (kAntiAliasing);
			if (item->isSeparator ())
			{
				context->setFillColor (theme.separatorColor);
				auto r = size;
				r.inset (0, r.getHeight () / 2);
				r.setHeight (1.);
				context->drawRect (r, kDrawFilled);
				return;
			}
			context->saveGlobalState ();
			if (flags & kRowSelected)
			{
				context->setFillColor (theme.selectedBackgroundColor);
				context->drawRect (size, kDrawFilled);
				context->setFontColor (theme.selectedTextColor);
			}
			else
			{
				CColor c = item->isTitle () ?
				               theme.titleTextColor :
				               item->isEnabled () ? theme.textColor : theme.disabledTextColor;
				context->setFontColor (c);
			}
			if (item->isTitle ())
				context->setFont (theme.font, 0, kBoldFace);
			else
				context->setFont (theme.font);
			if (item->isChecked ())
			{
				auto r = size;
				r.setWidth (getCheckmarkWidth ());
				drawCheckMark (context, r, flags & kRowSelected);
			}
			auto r = size;
			CHoriTxtAlign textAlign = kLeftText;
			if (item->isTitle ())
			{
				textAlign = kCenterText;
			}
			else
			{
				r.left += getCheckmarkWidth ();
				r.setWidth (maxTitleWidth);
			}
			{
				ConcatClip cc (*context, r);
				context->drawString (item->getTitle ().getPlatformString (), r, textAlign);
			}
			r.right = size.right - getCheckmarkWidth () / 2.;
			r.left = r.right - getSubmenuIndicatorWidth ();
			if (item->getSubmenu ())
			{
				drawSubmenuIndicator (context, r, flags & kRowSelected);
			}
			else if (auto icon = item->getIcon ())
			{
				drawItemIcon (context, r, icon);
			}
			context->restoreGlobalState ();
		}
	}

	CCoord getCheckmarkWidth ()
	{
		if (checkmarkSize == 0.)
			checkmarkSize = theme.font->getSize () * 1.6;
		return checkmarkSize;
	}
	CCoord getSubmenuIndicatorWidth () { return dbGetHeaderHeight (nullptr); }

	CViewContainer* mainContainer;
	COptionMenu* menu;
	CDataBrowser* db {nullptr};
	CView* subMenuView {nullptr};
	DataSource* parentDataSource {nullptr};
	ClickCallback clickCallback;
	CCoord checkmarkSize {0.};
	CCoord maxWidth {-1.};
	CCoord maxTitleWidth {-1.};
	int32_t selectedRow {-1};
	bool hasRightMargin {false};
	GenericOptionMenuTheme theme;
};

//------------------------------------------------------------------------
inline CColor makeDarkerColor (CColor baseColor)
{
	auto color = baseColor;
	double h, s, l;
	color.toHSL (h, s, l);
	l *= 0.7;
	color.fromHSL (h, s, l);
	return color;
}

//------------------------------------------------------------------------
template <typename Proc>
CView* setupGenericOptionMenu (Proc clickCallback, CViewContainer* container,
                               COptionMenu* optionMenu, GenericOptionMenuTheme& theme,
                               CRect viewRect, DataSource* parentDataSource)
{
	auto frame = container->getFrame ();
	auto dataSource =
	    makeOwned<DataSource> (container, optionMenu, clickCallback, theme, parentDataSource);
	auto maxWidth = dataSource->calculateMaxWidth (frame);
	if (parentDataSource)
	{
		viewRect.offset (viewRect.getWidth (), 0);
		viewRect.setWidth (maxWidth);
	}
	else if (optionMenu->isPopupStyle ())
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
	{
		viewRect.setWidth (maxWidth);
	}
	if (container)
	{
		auto frSize = container->getViewSize ();
		frSize.inset (theme.inset);

		if (frSize.bottom < viewRect.bottom)
		{
			viewRect.offset (0, frSize.bottom - viewRect.bottom);
		}
		if (frSize.top > viewRect.top)
		{
			viewRect.offset (0, frSize.top - viewRect.top);
		}
		if (frSize.right < viewRect.right)
		{
			viewRect.offset (frSize.right - viewRect.right, 0);
		}
		if (frSize.left > viewRect.left)
		{
			viewRect.offset (frSize.left - viewRect.left, 0);
		}
		viewRect.bound (frSize);
		if (maxWidth > viewRect.getWidth ())
			dataSource->setMaxWidth (viewRect.getWidth ());
	}
	viewRect.makeIntegral ();
	viewRect.inset (-1, -1);
	viewRect.offset (1, 1);
	auto decorView = new CViewContainer (viewRect);
	decorView->setBackgroundColor (
	    GenericOptionMenuDetail::makeDarkerColor (theme.backgroundColor));
	decorView->setBackgroundColorDrawStyle (kDrawStroked);
	viewRect.originize ();
	viewRect.inset (1., 1.);
	auto browser =
	    new CDataBrowser (viewRect, dataSource,
	                      CDataBrowser::kDontDrawFrame | CDataBrowser::kVerticalScrollbar |
	                          CDataBrowser::kOverlayScrollbars,
	                      2);
	if (auto sv = browser->getVerticalScrollbar ())
	{
		sv->setBackgroundColor (kTransparentCColor);
		sv->setFrameColor (kTransparentCColor);
		sv->setScrollerColor (theme.textColor);
	}
	browser->setBackgroundColor (theme.backgroundColor);
	decorView->addView (browser);

	container->addView (decorView);

	if (frame)
		frame->setFocusView (browser);

	using namespace Animation;
	decorView->setAlphaValue (0.f);
	decorView->addAnimation ("AlphaAnimation", new AlphaValueAnimation (1.f, true),
	                         new CubicBezierTimingFunction (
	                             CubicBezierTimingFunction::easyIn (theme.menuAnimationTime / 2)));
	if (!parentDataSource && optionMenu->isCheckStyle ())
	{
		browser->makeRowVisible (static_cast<int32_t> (optionMenu->getValue ()));
	}
	return decorView;
}

//------------------------------------------------------------------------
}

//------------------------------------------------------------------------
struct GenericOptionMenu::Impl
{
	using ContainerT = CLayeredViewContainer;
	SharedPointer<CFrame> frame;
	SharedPointer<COptionMenu> menu;
	SharedPointer<ContainerT> container;
	SharedPointer<CVSTGUITimer> mouseUpTimer;
	Optional<ModalViewSessionID> modalViewSession;
	IGenericOptionMenuListener* listener {nullptr};
	GenericOptionMenuTheme theme;
	Callback callback;
	CButtonState initialButtons;
	bool focusDrawingWasEnabled {false};
};

//------------------------------------------------------------------------
GenericOptionMenu::GenericOptionMenu (CFrame* frame, CButtonState initialButtons,
                                      GenericOptionMenuTheme theme)
{
	auto frameSize = frame->getViewSize ();
	frame->getTransform ().inverse ().transform (frameSize);
	frameSize.originize ();

	impl = std::unique_ptr<Impl> (new Impl);
	impl->frame = frame;
	impl->initialButtons = initialButtons;
	impl->theme = theme;
	impl->container = new Impl::ContainerT (frameSize);
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
	using namespace Animation;
	if (impl->callback)
	{
		if (impl->listener)
			impl->listener->optionMenuPopupStopped ();

		auto self = shared (this);
		impl->container->addAnimation (
		    "OptionMenuDone", new AlphaValueAnimation (0.f, true),
		    new CubicBezierTimingFunction (
		        CubicBezierTimingFunction::easyOut (impl->theme.menuAnimationTime)),
		    [self, result] (CView*, const IdStringPtr, IAnimationTarget*) {
			    if (!self->impl->container)
				    return;
			    auto callback = std::move (self->impl->callback);
			    self->impl->callback = nullptr;
			    self->impl->container->unregisterViewMouseListener (self);
			    if (self->impl->modalViewSession)
			    {
					self->impl->frame->endModalViewSession (*self->impl->modalViewSession);
					self->impl->modalViewSession = {};
				}
			    callback (self->impl->menu, result);
			    self->impl->frame->setFocusView (self->impl->menu);
			    self->impl->container = nullptr;
		    });
	}
}

//------------------------------------------------------------------------
CMouseEventResult GenericOptionMenu::viewOnMouseDown (CView* view, CPoint pos, CButtonState buttons)
{
	if (auto container = view->asViewContainer ())
	{
		CViewContainer::ViewList views;
		if (container->getViewsAt (pos, views, GetViewOptions ().deep ().includeInvisible ()))
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
CMouseEventResult GenericOptionMenu::viewOnMouseUp (CView* view, CPoint pos, CButtonState buttons)
{
	if (impl->initialButtons.isLeftButton () && buttons.isLeftButton ())
	{
		if (auto container = view->asViewContainer ())
		{
			CViewContainer::ViewList views;
			if (container->getViewsAt (pos, views, GetViewOptions ().deep ().includeInvisible ()))
			{
				if (view->onMouseDown (pos, buttons) == kMouseEventHandled)
					view->onMouseUp (pos, buttons);
			}
			else
			{
				auto self = shared (this);
				self->removeModalView ({nullptr, -1});
				return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
			}
		}
	}
	return kMouseEventNotHandled;
}

//------------------------------------------------------------------------
void GenericOptionMenu::popup (COptionMenu* optionMenu, const Callback& callback)
{
	impl->menu = optionMenu;
	impl->callback = callback;

	auto self = shared (this);
	auto clickCallback = [self] (COptionMenu* menu, int32_t index) {
		self->impl->container->unregisterViewMouseListener (self);
		self->removeModalView ({menu, index});
	};

	auto viewRect = optionMenu->translateToGlobal (optionMenu->getViewSize (), true);
	auto where = viewRect.getCenter ();

	GenericOptionMenuDetail::setupGenericOptionMenu (clickCallback, impl->container, optionMenu,
	                                                 impl->theme, viewRect, nullptr);

	if (auto view = impl->frame->getViewAt (where, GetViewOptions ().deep ().includeInvisible ()))
	{
		if (impl->initialButtons.getButtonState () != 0)
		{
			impl->frame->getCurrentMouseLocation (where);
			view->translateToLocal (where);
			view->onMouseMoved (where, impl->initialButtons);
			impl->mouseUpTimer = makeOwned<CVSTGUITimer> (
			    [this, where, view] (CVSTGUITimer* timer) {
				    timer->stop ();
				    if (!impl->container ||
				        impl->frame->getCurrentMouseButtons ().getButtonState () == 0)
					    return;
				    impl->container->registerViewMouseListener (this);
				    CPoint p (where);
				    view->translateToGlobal (p);
				    impl->frame->onMouseDown (p, impl->initialButtons);
			    },
			    200);
		}
	}
	if (impl->listener)
		impl->listener->optionMenuPopupStarted ();
}

//------------------------------------------------------------------------
} // VSTGUI
