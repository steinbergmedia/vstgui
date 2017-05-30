// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "vstgui/standalone/include/helpers/value.h"
#include "vstgui/standalone/include/helpers/valuelistener.h"
#include "vstgui/standalone/include/iuidescwindow.h"

namespace Mandelbrot {

//------------------------------------------------------------------------
struct ModelBinding : VSTGUI::Standalone::UIDesc::IModelBinding,
                      VSTGUI::Standalone::ValueListenerAdapter,
                      IModelChangeListener
{
	using Ptr = std::shared_ptr<ModelBinding>;
	using IValue = VSTGUI::Standalone::IValue;
	using ValuePtr = VSTGUI::Standalone::ValuePtr;
	using ValueConverterPtr = VSTGUI::Standalone::ValueConverterPtr;

	static Ptr make (Model::Ptr model) { return std::make_shared<ModelBinding> (model); }

	ModelBinding (Model::Ptr model) : model (model)
	{
		using namespace VSTGUI::Standalone;
		Value::performSingleStepEdit (*maxIterations.get (), model->getIterations ());

		values.emplace_back (maxIterations);
		values.emplace_back (minX);
		values.emplace_back (minY);
		values.emplace_back (maxX);
		values.emplace_back (maxY);
		values.emplace_back (progressValue);
		values.emplace_back (showParams);

		maxIterations->registerListener (this);
		minX->registerListener (this);
		minY->registerListener (this);
		maxX->registerListener (this);
		maxY->registerListener (this);

		model->registerListener (this);
	}

	const ValueList& getValues () const override { return values; }

	void modelChanged (const Model& model) override
	{
		using namespace VSTGUI::Standalone;
		Value::performSingleStepEdit (*maxIterations.get (), model.getIterations ());
		Value::performSinglePlainEdit (*minX.get (), model.getMin ().x);
		Value::performSinglePlainEdit (*minY.get (), model.getMin ().y);
		Value::performSinglePlainEdit (*maxX.get (), model.getMax ().x);
		Value::performSinglePlainEdit (*maxY.get (), model.getMax ().y);
	}

	void onPerformEdit (IValue& value, IValue::Type newValue) override
	{
		using namespace VSTGUI::Standalone;
		if (&value == maxIterations.get ())
		{
			auto step = Value::currentStepValue (*maxIterations.get ());
			if (step != IStepValue::InvalidStep)
				model->setIterations (step);
			return;
		}
		auto min = model->getMin ();
		auto max = model->getMax ();
		if (&value == maxX.get ())
			max.x = Value::currentPlainValue (value);
		else if (&value == maxY.get ())
			max.y = Value::currentPlainValue (value);
		else if (&value == minX.get ())
			min.x = Value::currentPlainValue (value);
		else if (&value == minY.get ())
			min.y = Value::currentPlainValue (value);
		model->setMinMax (min, max);
	}

	const ValuePtr& getProgressValue () const { return progressValue; }
	const ValuePtr& getMaxIterationsValue () const { return maxIterations; }
private:
	static constexpr auto numMaxIterations = 2048.;
	ValueConverterPtr xConverter {VSTGUI::Standalone::Value::makeRangeConverter (-2.2, 1.2)};
	ValueConverterPtr yConverter {VSTGUI::Standalone::Value::makeRangeConverter (-1.7, 1.7)};

	ValuePtr maxIterations {VSTGUI::Standalone::Value::makeStepValue ("max interations", numMaxIterations)};
	ValuePtr minX {VSTGUI::Standalone::Value::make ("minX", 0., xConverter)};
	ValuePtr minY {VSTGUI::Standalone::Value::make ("minY", 0., yConverter)};
	ValuePtr maxX {VSTGUI::Standalone::Value::make ("maxX", 1., xConverter)};
	ValuePtr maxY {VSTGUI::Standalone::Value::make ("maxY", 1., yConverter)};
	ValuePtr progressValue {VSTGUI::Standalone::Value::make ("progress")};
	ValuePtr showParams {VSTGUI::Standalone::Value::make ("showParams")};
	ValueList values;

	Model::Ptr model;
};
}
