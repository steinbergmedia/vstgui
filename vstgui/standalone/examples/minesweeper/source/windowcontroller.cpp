// This file is part of VSTGUI. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "commands.h"
#include "highscores.h"
#include "highscoreviewcontroller.h"
#include "keepchildviewscentered.h"
#include "minefieldviewcontroller.h"
#include "vstgui/standalone/include/helpers/preferences.h"
#include "vstgui/standalone/include/helpers/value.h"
#include "vstgui/standalone/include/iapplication.h"
#include "vstgui/uidescription/delegationcontroller.h"
#include "vstgui/uidescription/iuidescription.h"
#include "vstgui/uidescription/uiattributes.h"
#include "windowcontroller.h"
#include <cassert>

//------------------------------------------------------------------------
namespace VSTGUI {
namespace Standalone {
namespace Minesweeper {

//------------------------------------------------------------------------
class EnterHighScoreViewController final : public DelegationController,
                                           public ValueListenerAdapter,
                                           public NonAtomicReferenceCounted
{
public:
	using OnEndEditFunc = std::function<void ()>;

	EnterHighScoreViewController (IValue& nameValue, IValue& okValue, IController* parent,
	                              OnEndEditFunc&& func)
	: DelegationController (parent)
	, nameValue (nameValue)
	, okValue (okValue)
	, onEndEditFunc (std::move (func))
	{
		okValue.registerListener (this);
		okValue.setActive (false);
	}

	CView* verifyView (CView* view, const UIAttributes& attributes,
	                   const IUIDescription* description) override
	{
		const auto attr = attributes.getAttributeValue (IUIDescription::kCustomViewName);
		if (attr)
		{
			if (*attr == "MainView")
			{
				mainView = view;
				mainView->setAlphaValue (0.f);
				mainView->setMouseEnabled (false);
				keepChildViewsCentered (mainView->asViewContainer ());
			}
		}

		return DelegationController::verifyView (view, attributes, description);
	}

	void onNewHighscore (uint32_t secondsToWin, const std::shared_ptr<HighScoreList>& list)
	{
		currentSecondsToWin = secondsToWin;
		highscoreList = list;
		mainView->setAlphaValue (1.f);
		mainView->setMouseEnabled (true);
		okValue.setActive (true);
	}

	void onEndEdit (IValue& value) override
	{
		if (value.getValue () > 0.5)
		{
			value.performEdit (0.);
			auto name = Value::currentStringValue (nameValue);
			highscoreList->addHighscore (name, currentSecondsToWin);
			highscoreList = nullptr;
			mainView->setMouseEnabled (false);
			mainView->setAlphaValue (0.f);
			onEndEditFunc ();
		}
	}

private:
	SharedPointer<CView> mainView;
	IValue& nameValue;
	IValue& okValue;
	std::shared_ptr<HighScoreList> highscoreList;
	uint32_t currentSecondsToWin {0};
	OnEndEditFunc onEndEditFunc;
};

//------------------------------------------------------------------------
class DigitsDisplayConverter final : public IValueConverter
{
public:
	DigitsDisplayConverter (ValueConverterPtr converter, uint32_t digits)
	: converter (converter), digits (digits)
	{
	}

	UTF8String valueAsString (IValue::Type value) const override
	{
		auto plain = static_cast<uint32_t> (std::round (converter->normalizedToPlain (value)));
		UTF8String str;
		auto val = static_cast<uint32_t> (std::pow (10, digits - 1));
		for (auto i = digits; i > 0; --i)
		{
			auto v = plain / val;
			str += toString (v);
			plain -= v * val;
			val /= 10;
		}
		return str;
	}
	IValue::Type stringAsValue (const UTF8String& string) const override
	{
		return converter->stringAsValue (string);
	}
	IValue::Type plainToNormalized (IValue::Type plain) const override
	{
		return converter->plainToNormalized (plain);
	}
	IValue::Type normalizedToPlain (IValue::Type normalized) const override
	{
		return converter->normalizedToPlain (normalized);
	}

private:
	ValueConverterPtr converter;
	uint32_t digits {3};
};

//------------------------------------------------------------------------
static constexpr auto valueRows = "Rows";
static constexpr auto valueCols = "Cols";
static constexpr auto valueMines = "Mines";
static constexpr auto valueStart = "Start";
static constexpr auto valueFlags = "Flags";
static constexpr auto valueTime = "Time";
static constexpr auto valueMouseMode = "MouseMode";
static constexpr auto valueNewHighScoreOK = "NewHighScoreOK";
static constexpr auto valueNewHighScoreName = "NewHighScoreName";
static constexpr auto valueCloseHighscoreView = "CloseHighscoreView";

//------------------------------------------------------------------------
WindowController::WindowController ()
{
	IApplication::instance ().registerCommand (NewGameCommand, 'n');
	IApplication::instance ().registerCommand (NewBeginnerGameCommand, '1');
	IApplication::instance ().registerCommand (NewIntermediateGameCommand, '2');
	IApplication::instance ().registerCommand (NewExpertGameCommand, '3');
	IApplication::instance ().registerCommand (MouseModeCommand, 0);
	IApplication::instance ().registerCommand (TouchpadModeCommand, 0);
	IApplication::instance ().registerCommand (ToggleHighscoresCommand, '/');

	addCreateViewControllerFunc (
	    "MinefieldController", [this] (const auto& name, auto* parent, auto* uidesc) {
		    if (!minefieldViewController)
		    {
			    auto flagsValue = modelBinding.getValue (valueFlags);
			    auto timeValue = modelBinding.getValue (valueTime);
			    minefieldViewController = owned (new MinefieldViewController (
			        *flagsValue, *timeValue, parent,
			        [this] (uint32_t secondsToWin) { onWon (secondsToWin); }));
			    if (auto valueObject = modelBinding.getValue (valueMouseMode))
			    {
				    minefieldViewController->setMouseMode (valueObject->getValue () >= 0.5 ? true :
				                                                                             false);
			    }
		    }
		    minefieldViewController->remember ();
		    return minefieldViewController;
	    });

	addCreateViewControllerFunc (
	    "NewHighScoreViewController", [this] (const auto& name, auto* parent, auto* uidesc) {
		    if (!enterHighscoreViewController)
		    {
			    auto nameValue = modelBinding.getValue (valueNewHighScoreName);
			    auto okValue = modelBinding.getValue (valueNewHighScoreOK);
			    enterHighscoreViewController = owned (new EnterHighScoreViewController (
			        *nameValue, *okValue, parent, [this] () { showHighscores (); }));
		    }
		    enterHighscoreViewController->remember ();
		    return enterHighscoreViewController;
	    });

	addCreateViewControllerFunc (
	    "HighScoreViewController", [this] (const auto& name, auto* parent, auto* uidesc) {
		    if (!highscoreViewController)
			    highscoreViewController = owned (new HighScoreViewController (parent));
		    highscoreViewController->remember ();
		    return highscoreViewController;
	    });

	modelBinding.addValue (
	    Value::make (valueRows, 0, Value::makeRangeConverter (8, 30, 0)),
	    UIDesc::ValueCalls::onEndEdit ([this] (auto& value) { verifyNumMines (); }));
	modelBinding.addValue (
	    Value::make (valueCols, 0, Value::makeRangeConverter (8, 30, 0)),
	    UIDesc::ValueCalls::onEndEdit ([this] (auto& value) { verifyNumMines (); }));
	modelBinding.addValue (
	    Value::make (valueMines, 0, Value::makeRangeConverter (4, 668, 0)),
	    UIDesc::ValueCalls::onEndEdit ([this] (auto& value) { verifyNumMines (); }));
	modelBinding.addValue (Value::make (
	    valueFlags, 0,
	    std::make_shared<DigitsDisplayConverter> (Value::makeRangeConverter (0, 668, 0), 2)));
	modelBinding.addValue (
	    Value::make (valueTime, 0,
	                 std::make_shared<DigitsDisplayConverter> (
	                     Value::makeRangeConverter (0, maxTimeInSeconds, 0), 3)));
	if (auto value = modelBinding.getValue (valueMines))
		Value::performSinglePlainEdit (*value.get (), 10);
	modelBinding.addValue (
	    Value::make (valueStart),
	    UIDesc::ValueCalls::onEndEdit ([this] (auto& value) { startNewGame (); }));
	modelBinding.addValue (
	    Value::make (valueMouseMode), UIDesc::ValueCalls::onPerformEdit ([this] (auto& value) {
		    if (minefieldViewController)
			    minefieldViewController->setMouseMode (value.getValue () >= 0.5 ? true : false);
	    }));

	modelBinding.addValue (Value::makeStringValue (valueNewHighScoreName, ""));
	modelBinding.addValue (Value::make (valueNewHighScoreOK));

	modelBinding.addValue (Value::make (valueCloseHighscoreView),
	                       UIDesc::ValueCalls::onPerformEdit ([this] (auto& value) {
		                       if (value.getValue () >= 0.5)
		                       {
			                       value.performEdit (0.);
			                       hideHighscores ();
		                       }
	                       }));

	loadDefaults ();
}

//------------------------------------------------------------------------
WindowController::~WindowController () noexcept = default;

//------------------------------------------------------------------------
void WindowController::loadDefaults ()
{
	Preferences prefs ("Values");
	if (auto value = prefs.getNumber<int32_t> (valueRows))
	{
		if (auto valueObject = modelBinding.getValue (valueRows))
			Value::performSinglePlainEdit (*valueObject, *value);
	}
	if (auto value = prefs.getNumber<int32_t> (valueCols))
	{
		if (auto valueObject = modelBinding.getValue (valueCols))
			Value::performSinglePlainEdit (*valueObject, *value);
	}
	if (auto value = prefs.getNumber<int32_t> (valueMines))
	{
		if (auto valueObject = modelBinding.getValue (valueMines))
			Value::performSinglePlainEdit (*valueObject, *value);
	}
	if (auto value = prefs.getNumber<bool> (valueMouseMode))
	{
		if (auto valueObject = modelBinding.getValue (valueMouseMode))
			Value::performSinglePlainEdit (*valueObject, *value);
	}
}

//------------------------------------------------------------------------
void WindowController::storeDefaults ()
{
	Preferences prefs ("Values");
	if (auto value = modelBinding.getValue (valueRows))
		prefs.setNumber (valueRows, static_cast<int32_t> (Value::currentPlainValue (*value)));
	if (auto value = modelBinding.getValue (valueCols))
		prefs.setNumber (valueCols, static_cast<int32_t> (Value::currentPlainValue (*value)));
	if (auto value = modelBinding.getValue (valueMines))
		prefs.setNumber (valueMines, static_cast<int32_t> (Value::currentPlainValue (*value)));
	if (auto value = modelBinding.getValue (valueMouseMode))
		prefs.setNumber (valueMouseMode, static_cast<bool> (Value::currentPlainValue (*value)));
}

//------------------------------------------------------------------------
bool WindowController::canHandleCommand (const Command& command)
{
	if (command.group == GameGroup)
	{
		if (command.name == TouchpadMode)
		{
			if (modelBinding.getValue (valueMouseMode)->getValue () < 0.5)
				return false;
		}
		if (command.name == MouseMode)
		{
			if (modelBinding.getValue (valueMouseMode)->getValue () >= 0.5)
				return false;
		}
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
bool WindowController::handleCommand (const Command& command)
{
	if (command.group != GameGroup)
		return false;
	if (command.name == ToggleHighscores)
	{
		showHideHighscores ();
		return true;
	}
	if (command.name == TouchpadMode)
	{
		auto value = modelBinding.getValue (valueMouseMode);
		Value::performSingleEdit (*value, 0.);
		return true;
	}
	else if (command.name == MouseMode)
	{
		auto value = modelBinding.getValue (valueMouseMode);
		Value::performSingleEdit (*value, 1.);
		return true;
	}

	auto rowValue = modelBinding.getValue (valueRows);
	auto colValue = modelBinding.getValue (valueCols);
	auto mineValue = modelBinding.getValue (valueMines);
	if (!rowValue || !colValue || !mineValue)
		return false;
	if (command == NewBeginnerGameCommand)
	{
		Value::performSinglePlainEdit (*rowValue, 9);
		Value::performSinglePlainEdit (*colValue, 9);
		Value::performSinglePlainEdit (*mineValue, 10);
	}
	else if (command == NewIntermediateGameCommand)
	{
		Value::performSinglePlainEdit (*rowValue, 16);
		Value::performSinglePlainEdit (*colValue, 16);
		Value::performSinglePlainEdit (*mineValue, 40);
	}
	else if (command == NewExpertGameCommand)
	{
		Value::performSinglePlainEdit (*rowValue, 16);
		Value::performSinglePlainEdit (*colValue, 30);
		Value::performSinglePlainEdit (*mineValue, 99);
	}
	else
	{
		hideHighscores ();
	}

	startNewGame ();
	return true;
}

//------------------------------------------------------------------------
void WindowController::beforeShow (IWindow& w)
{
	window = &w;
}

//------------------------------------------------------------------------
void WindowController::onShow (const IWindow& w)
{
	startNewGame ();
}

//------------------------------------------------------------------------
void WindowController::onClosed (const IWindow& w)
{
	storeDefaults ();
}

//------------------------------------------------------------------------
auto WindowController::getValues () const -> const ValueList&
{
	return modelBinding.getValues ();
}

//------------------------------------------------------------------------
std::tuple<uint32_t, uint32_t, uint32_t> WindowController::getRowsColsMines ()
{
	auto rows =
	    static_cast<uint32_t> (Value::currentPlainValue (*modelBinding.getValue (valueRows)));
	auto cols =
	    static_cast<uint32_t> (Value::currentPlainValue (*modelBinding.getValue (valueCols)));
	auto mines =
	    static_cast<uint32_t> (Value::currentPlainValue (*modelBinding.getValue (valueMines)));
	return {rows, cols, mines};
}

//------------------------------------------------------------------------
void WindowController::startNewGame ()
{
	assert (minefieldViewController);
	uint32_t rows, cols, mines;
	std::tie (rows, cols, mines) = getRowsColsMines ();
	minefieldViewController->startGame (rows, cols, mines);
	auto highscoreList = HighScores::instance ().get (rows, cols, mines);
	if (!highscoreList)
		return;
	highscoreViewController->setHighScoreList (highscoreList);
}

//------------------------------------------------------------------------
void WindowController::verifyNumMines ()
{
	uint32_t rows, cols, mines;
	std::tie (rows, cols, mines) = getRowsColsMines ();
	if (rows * cols < mines)
	{
		Value::performSinglePlainEdit (*modelBinding.getValue (valueMines), rows * cols * 0.8);
	}
}

//------------------------------------------------------------------------
void WindowController::onWon (uint32_t secondsToWin)
{
	uint32_t rows, cols, mines;
	std::tie (rows, cols, mines) = getRowsColsMines ();
	auto highscoreList = HighScores::instance ().get (rows, cols, mines);
	if (!highscoreList)
		return;

	auto pos = highscoreList->isHighScore (secondsToWin);
	if (pos)
	{
		enterHighscoreViewController->onNewHighscore (secondsToWin, highscoreList);
	}
}

//------------------------------------------------------------------------
void WindowController::showHideHighscores ()
{
	if (!highscoreViewController)
		return;
	if (highscoreViewController->isVisible ())
		highscoreViewController->hide ();
	else
		showHighscores ();
}

//------------------------------------------------------------------------
void WindowController::showHighscores ()
{
	assert (highscoreViewController);
	uint32_t rows, cols, mines;
	std::tie (rows, cols, mines) = getRowsColsMines ();
	if (auto highscoreList = HighScores::instance ().get (rows, cols, mines))
	{
		highscoreViewController->setHighScoreList (highscoreList);
		highscoreViewController->show ();
	}
}

//------------------------------------------------------------------------
void WindowController::hideHighscores ()
{
	assert (highscoreViewController);
	highscoreViewController->hide ();
}

//------------------------------------------------------------------------
} // Minesweeper
} // Standalone
} // VSTGUI
