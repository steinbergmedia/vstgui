// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "minefieldviewcontroller.h"
#include "vstgui/lib/animation/animations.h"
#include "vstgui/lib/animation/timingfunctions.h"
#include "vstgui/lib/cdatabrowser.h"
#include "vstgui/lib/cdrawcontext.h"
#include "vstgui/lib/cvstguitimer.h"
#include "vstgui/standalone/include/helpers/value.h"
#include "vstgui/uidescription/iuidescription.h"
#include "vstgui/uidescription/uiattributes.h"

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Minesweeper {

MinefieldViewController::MinefieldViewController (IValue& flagsValue, IValue& timeValue,
                                                  IController* parent,
                                                  WonCallbackFunc&& wonCallback)
: DelegationController (parent)
, flagsValue (flagsValue)
, timeValue (timeValue)
, wonCallback (std::move (wonCallback))
{
}

//------------------------------------------------------------------------
void MinefieldViewController::startGame (uint32_t rows, uint32_t cols, uint32_t mines)
{
	if (lostView)
	{
		lostView->removeAllAnimations ();
		lostView->setAlphaValue (0.f);
	}
	if (wonView)
	{
		wonView->removeAllAnimations ();
		wonView->setAlphaValue (0.f);
	}
	model = std::make_unique<Model> (rows, cols, mines, this);
	numRows = rows;
	numCols = cols;
	updateCellSize (dataBrowser->getViewSize ().getSize ());
	Value::performSinglePlainEdit (flagsValue, model->getNumberOfMines ());
	Value::performSinglePlainEdit (timeValue, 0);
	startTime = {};
}

//------------------------------------------------------------------------
void MinefieldViewController::setMouseMode (bool state)
{
	mouseMode = state;
}

//------------------------------------------------------------------------
CView* MinefieldViewController::createView (const UIAttributes& attributes,
                                            const IUIDescription* description)
{
	const auto attr = attributes.getAttributeValue (IUIDescription::kCustomViewName);
	if (attr && *attr == "MinefieldView")
	{
		description->getColor ("card.closed.frame", closedFrameColor);
		description->getColor ("card.closed.back", closedBackColor);
		description->getColor ("card.opened.frame", openedFrameColor);
		description->getColor ("card.opened.back", openedBackColor);
		description->getColor ("card.flaged.frame", flagedFrameColor);
		description->getColor ("card.flaged.back", flagedBackColor);
		if (auto f = description->getFont ("emoji"))
			emojiFont = *f;
		smallEmojiFont = emojiFont;
		if (dataBrowser)
			dataBrowser->unregisterViewListener (this);
		dataBrowser = nullptr;
		dataBrowser = new CDataBrowser ({}, this, 0, 0.);
		dataBrowser->registerViewListener (this);
		return dataBrowser;
	}
	return DelegationController::createView (attributes, description);
}

//------------------------------------------------------------------------
CView* MinefieldViewController::verifyView (CView* view, const UIAttributes& attributes,
                                            const IUIDescription* description)
{
	const auto attr = attributes.getAttributeValue (IUIDescription::kCustomViewName);
	if (attr)
	{
		if (*attr == "LostView")
		{
			lostView = view;
			lostView->setAlphaValue (0.f);
		}
		else if (*attr == "WonView")
		{
			wonView = view;
			wonView->setAlphaValue (0.f);
		}
	}

	return DelegationController::verifyView (view, attributes, description);
}

//------------------------------------------------------------------------
void MinefieldViewController::onCellChanged (uint32_t row, uint32_t col)
{
	if (!dataBrowser)
		return;
	auto r = dataBrowser->getCellBounds (CDataBrowser::Cell (row, col));
	r.extend (1., 1.);
	dataBrowser->invalidRect (r);
}

//------------------------------------------------------------------------
int32_t MinefieldViewController::dbGetNumRows (CDataBrowser* browser)
{
	return numRows;
}

//------------------------------------------------------------------------
int32_t MinefieldViewController::dbGetNumColumns (CDataBrowser* browser)
{
	return numCols;
}

//------------------------------------------------------------------------
CCoord MinefieldViewController::dbGetRowHeight (CDataBrowser* browser)
{
	return cellSize.y;
}

//------------------------------------------------------------------------
CCoord MinefieldViewController::dbGetCurrentColumnWidth (int32_t index, CDataBrowser* browser)
{
	return cellSize.x;
}

//------------------------------------------------------------------------
bool MinefieldViewController::dbGetLineWidthAndColor (CCoord& width, CColor& color,
                                                      CDataBrowser* browser)
{
	width = 1;
	color = kBlackCColor;
	return true;
}

//------------------------------------------------------------------------
void MinefieldViewController::drawClosedCell (const CRect& r, CDrawContext& context) const
{
	context.setFrameColor (closedFrameColor);
	context.setFillColor (closedBackColor);
	context.drawRect (r, kDrawFilledAndStroked);
}

//------------------------------------------------------------------------
void MinefieldViewController::drawOpenCell (const CRect& r, CDrawContext& context) const
{
	context.setFrameColor (openedFrameColor);
	context.setFillColor (openedBackColor);
	context.drawRect (r, kDrawFilledAndStroked);
}

//------------------------------------------------------------------------
void MinefieldViewController::drawQuestionMark (const CRect& r, CDrawContext& context,
                                                CFontRef f) const
{
	context.setFont (f);
	context.setFontColor (kRedCColor);
	context.drawString (QuestionMarkCharacter, r);
}

//------------------------------------------------------------------------
void MinefieldViewController::drawQuestionMarkCell (const CRect& r, CDrawContext& context,
                                                    CFontRef f) const
{
	context.setFrameColor (flagedFrameColor);
	context.setFillColor (flagedBackColor);
	context.drawRect (r, kDrawFilledAndStroked);
	drawQuestionMark (r, context, f);
}

//------------------------------------------------------------------------
void MinefieldViewController::drawFlag (const CRect& r, CDrawContext& context, CFontRef f) const
{
	context.setFont (f);
	context.setFontColor (kRedCColor);
	context.drawString (FlagCharacter, r);
}

//------------------------------------------------------------------------
void MinefieldViewController::drawFlaggedCell (const CRect& r, CDrawContext& context,
                                               CFontRef f) const
{
	context.setFrameColor (flagedFrameColor);
	context.setFillColor (flagedBackColor);
	context.drawRect (r, kDrawFilledAndStroked);
	drawFlag (r, context, f);
}

//------------------------------------------------------------------------
void MinefieldViewController::drawMinedCell (const CRect& r, CDrawContext& context,
                                             CFontRef f) const
{
	context.setFont (f);
	context.setFontColor (kBlackCColor);
	context.drawString (BombCharacter, r);
}

//------------------------------------------------------------------------
void MinefieldViewController::drawExplosionCell (const CRect& r, CDrawContext& context,
                                                 CFontRef f) const
{
	context.setFont (f);
	context.setFontColor (kRedCColor);
	context.drawString (ExplosionCharacter, r);
}

//------------------------------------------------------------------------
void MinefieldViewController::drawCellNeighbours (const CRect& r, CDrawContext& context, CFontRef f,
                                                  uint32_t neighbours)
{
	if (neighbours == 0)
		return;
	auto valueStr = toString (neighbours);
	context.setFont (f);
	context.setFontColor (kBlackCColor);
	context.drawString (valueStr, r);
}

//------------------------------------------------------------------------
void MinefieldViewController::dbDrawCell (CDrawContext* context, const CRect& size, int32_t row,
                                          int32_t column, int32_t flags, CDataBrowser* browser)
{
	if (row < 0 || column < 0 || !model)
		return;
	context->setDrawMode (kAntiAliasing);
	context->setLineWidth (1.);
	CRect r (size);
	r.inset (1.5, 1.5);
	if (!model->isDone () && !model->isTrapped () && !model->isOpen (row, column))
	{
		if (model->isFlag (row, column))
		{
			drawFlaggedCell (r, *context, &emojiFont);
		}
		else if (model->isQuestion (row, column))
		{
			drawQuestionMarkCell (r, *context, &emojiFont);
		}
		else
		{
			drawClosedCell (r, *context);
		}
		return;
	}
	drawOpenCell (r, *context);
	if (model->isMine (row, column))
	{
		if (model->isTrapMine (row, column))
			drawExplosionCell (r, *context, &emojiFont);
		else
			drawMinedCell (r, *context, &emojiFont);
	}
	else
	{
		auto value = model->getNumberOfMinesNearby (row, column);
		drawCellNeighbours (r, *context, &font, value);
	}
	if (model->isFlag (row, column))
	{
		r.setWidth (r.getWidth () / 2.);
		r.setHeight (r.getHeight () / 2.);
		drawFlag (r, *context, &smallEmojiFont);
	}
}

//------------------------------------------------------------------------
CMouseEventResult MinefieldViewController::dbOnMouseDown (const CPoint& where,
                                                          const CButtonState& buttons, int32_t row,
                                                          int32_t column, CDataBrowser* browser)
{
	ignoreMouseUp = false;
	if (!mouseMode && buttons.isLeftButton ())
	{
		mouseDownTimer = makeOwned<CVSTGUITimer> (
		    [this, row, column] (auto) {
			    mouseDownTimer = nullptr;
			    if (!model->isOpen (row, column))
			    {
				    model->mark (row, column);
				    checkGameOver ();
			    }
			    ignoreMouseUp = true;
		    },
		    60);
	}
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
CMouseEventResult MinefieldViewController::dbOnMouseMoved (const CPoint& where,
                                                           const CButtonState& buttons, int32_t row,
                                                           int32_t column, CDataBrowser* browser)
{
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
CMouseEventResult MinefieldViewController::dbOnMouseUp (const CPoint& where,
                                                        const CButtonState& buttons, int32_t row,
                                                        int32_t column, CDataBrowser* browser)
{
	mouseDownTimer = nullptr;
	if (ignoreMouseUp || !model || row < 0 || column < 0 || model->isTrapped () || model->isDone ())
		return kMouseEventHandled;
	if (!model->isOpen (row, column))
	{
		if (buttons.isRightButton ())
		{
			model->mark (row, column);
		}
		else if (buttons.isLeftButton ())
		{
			if (!model->isFlag (row, column) && !model->isQuestion (row, column))
				model->open (row, column);
			else
				model->mark (row, column);
		}
		checkGameOver ();
	}
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
void MinefieldViewController::checkGameOver ()
{
	if (startTime == TimePoint {})
	{
		startTime = Clock::now ();
		gameTimer = makeOwned<CVSTGUITimer> ([this] (auto) { onTimer (); }, 1000);
	}
	Value::performSinglePlainEdit (flagsValue,
	                               model->getNumberOfMines () - model->getNumberOfFlags ());
	if (model->isDone () && !model->isTrapped ())
	{
		onGameWon ();
	}
	if (model->isTrapped ())
	{
		onGameLost ();
	}
}

//------------------------------------------------------------------------
void MinefieldViewController::onGameLost ()
{
	auto animView = lostView;
	if (auto vc = lostView->asViewContainer ())
	{
		animView = vc->getView (0);
		animView->setAlphaValue (0.f);
		lostView->setAlphaValue (1.f);
	}
	animView->addAnimation ("Lost", new Animation::AlphaValueAnimation (1.f),
	                        new Animation::RepeatTimingFunction (
	                            new Animation::CubicBezierTimingFunction (
	                                Animation::CubicBezierTimingFunction::easyInOut (250)),
	                            -1));
	gameTimer = nullptr;
	dataBrowser->invalid ();
}

//------------------------------------------------------------------------
void MinefieldViewController::onGameWon ()
{
	auto animView = wonView;
	if (auto vc = wonView->asViewContainer ())
	{
		animView = vc->getView (0);
		animView->setAlphaValue (0.f);
		wonView->setAlphaValue (1.f);
	}
	animView->addAnimation ("Won", new Animation::AlphaValueAnimation (1.f),
	                        new Animation::RepeatTimingFunction (
	                            new Animation::CubicBezierTimingFunction (
	                                Animation::CubicBezierTimingFunction::easyInOut (400)),
	                            -1));
	gameTimer = nullptr;
	dataBrowser->invalid ();
	if (wonCallback)
		wonCallback (static_cast<int32_t> (Value::currentPlainValue (timeValue)));
}

//------------------------------------------------------------------------
void MinefieldViewController::onTimer ()
{
	auto now = Clock::now ();
	auto distance = std::chrono::duration_cast<std::chrono::seconds> (now - startTime).count ();
	if (distance >= maxTimeInSeconds)
		gameTimer = nullptr;
	Value::performSinglePlainEdit (timeValue, static_cast<IValue::Type> (distance));
}

//------------------------------------------------------------------------
void MinefieldViewController::viewSizeChanged (CView* view, const CRect& oldSize)
{
	if (!dataBrowser)
		return;
	auto newSize = view->getViewSize ().getSize ();
	updateCellSize (newSize);
}

//------------------------------------------------------------------------
void MinefieldViewController::updateCellSize (CPoint newSize)
{
	newSize -= {3., 3.};
	cellSize.x = newSize.x / numCols;
	cellSize.y = newSize.y / numRows;
	dataBrowser->recalculateLayout ();
	font.setSize (cellSize.y / 2.);
	emojiFont.setSize (cellSize.y / 2.);
	smallEmojiFont.setSize (font.getSize () / 2.);
}

//------------------------------------------------------------------------
} // Minesweeper
} // Standalone
} // VSTGUI
