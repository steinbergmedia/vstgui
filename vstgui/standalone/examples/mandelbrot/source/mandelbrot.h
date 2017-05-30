// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "vstgui/lib/cpoint.h"
#include "vstgui/lib/dispatchlist.h"
#include <complex>
#include <iostream>
#include <memory>

//------------------------------------------------------------------------
namespace Mandelbrot {

using Point = VSTGUI::CPoint;
using Complex = std::complex<double>;
struct Model;

//------------------------------------------------------------------------
struct IModelChangeListener
{
	virtual void modelChanged (const Model& model) = 0;
};

//------------------------------------------------------------------------
struct Model
{
	using Ptr = std::shared_ptr<Model>;

	void registerListener (IModelChangeListener* listener) { listeners.add (listener); }
	void unregisterListener (IModelChangeListener* listener) { listeners.remove (listener); }

	const Point& getMax () const { return max; }
	const Point& getMin () const { return min; }
	uint32_t getIterations () const { return iterations; }

	void setIterations (uint32_t newIterations)
	{
		if (newIterations != iterations)
		{
			iterations = newIterations;
			changed ();
		}
	}

	void setMinMax (Point newMin, Point newMax)
	{
		if (max == newMax && min == newMin)
			return;
		max = newMax;
		min = newMin;
		changed ();
	}

private:
	void changed ()
	{
		listeners.forEach ([this] (auto& l) { l->modelChanged (*this); });
	}
	VSTGUI::DispatchList<IModelChangeListener*> listeners;

	Point max {1.2, 1.7};
	Point min {-2.2, -1.7};
	uint32_t iterations {50};
};

//------------------------------------------------------------------------
inline Point pixelToPoint (Point max, Point min, Point size, Point pixel)
{
	Point p;
	p.x = min.x + pixel.x / (size.x - 1.0) * (max.x - min.x);
	p.y = min.y + pixel.y / (size.y - 1.0) * (max.y - min.y);
	return p;
}

//------------------------------------------------------------------------
inline double hypot (double x, double y)
{
	return std::sqrt (x * x + y * y);
}

//------------------------------------------------------------------------
inline Complex mulAdd (Complex z, Complex w, Complex v)
{
	auto a = z.real ();
	auto b = z.imag ();
	auto c = w.real ();
	auto d = w.imag ();
	auto ac = a * c;
	auto bd = b * d;
	auto ad = a * d;
	auto bc = b * c;
	auto x = ac - bd;
	auto y = ad + bc;
	return Complex (x + v.real (), y + v.imag ());
}

//------------------------------------------------------------------------
template <typename SetPixelProc>
inline void calculateLine (uint32_t line, Point size, const Model& model, SetPixelProc setPixel)
{
	Point sizeInv (size);
	sizeInv -= {1, 1};
	sizeInv.x = 1. / sizeInv.x;
	sizeInv.y = 1. / sizeInv.y;
	Point diff;
	diff.x = model.getMax ().x - model.getMin ().x;
	diff.y = model.getMax ().y - model.getMin ().y;
	Point pos;
	pos.y = model.getMin ().y + line * sizeInv.y * diff.y;
	std::vector<uint32_t> iterationResult (size.x);
	for (auto x = 0u; x < size.x; ++x)
	{
		pos.x = model.getMin ().x + x * sizeInv.x * diff.x;
		Complex c {pos.x, pos.y};
		Complex z {0};
		uint32_t iterations {};

		for (; iterations < model.getIterations () && hypot (z.real (), z.imag ()) < 2.0;
		     ++iterations)
			z = mulAdd (z, z, c);
		iterationResult[x] = iterations;
	}
	for (auto x = 0u; x < size.x; ++x)
		setPixel (x, iterationResult[x]);
}

//------------------------------------------------------------------------
template <typename SetPixelProc>
inline void calculate (Point size, const Model& model, SetPixelProc setPixel)
{
	for (auto y = 0u; y < size.y; ++y)
	{
		calculateLine (y, size, model, setPixel);
	}
}

//------------------------------------------------------------------------
} // Mandelbrot
