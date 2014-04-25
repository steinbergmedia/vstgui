//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 4.2
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2013, Steinberg Media Technologies, All Rights Reserved
//-----------------------------------------------------------------------------
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
// 
//   * Redistributions of source code must retain the above copyright notice, 
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation 
//     and/or other materials provided with the distribution.
//   * Neither the name of the Steinberg Media Technologies nor the names of its
//     contributors may be used to endorse or promote products derived from this 
//     software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#include "cbitmapfilter.h"
#include "cbitmap.h"
#include "ccolor.h"
#include "cgraphicspath.h"

namespace VSTGUI {

namespace BitmapFilter {

//----------------------------------------------------------------------------------------------------
Property::Property (Type type)
: type (type)
, value (0)
{
}

//----------------------------------------------------------------------------------------------------
Property::Property (int32_t intValue)
: type (kInteger)
{
	value = std::malloc (sizeof (int32_t));
	memcpy (value, &intValue, sizeof (int32_t));
}

//----------------------------------------------------------------------------------------------------
Property::Property (double floatValue)
: type (kFloat)
{
	value = std::malloc (sizeof (double));
	memcpy (value, &floatValue, sizeof (double));
}

//----------------------------------------------------------------------------------------------------
Property::Property (CBaseObject* objectValue)
: type (kObject)
{
	value = std::malloc (sizeof (CBaseObject*));
	memcpy (value, &objectValue, sizeof (CBaseObject*));
	objectValue->remember ();
}

//----------------------------------------------------------------------------------------------------
Property::Property (const CRect& rectValue)
: type (kRect)
{
	value = std::malloc (sizeof (CRect));
	memcpy (value, &rectValue, sizeof (CRect));
}

//----------------------------------------------------------------------------------------------------
Property::Property (const CPoint& pointValue)
: type (kPoint)
{
	value = std::malloc (sizeof (CPoint));
	memcpy (value, &pointValue, sizeof (CPoint));
}

//----------------------------------------------------------------------------------------------------
Property::Property (const CColor& colorValue)
: type (kColor)
{
	value = std::malloc (sizeof (CColor));
	memcpy (value, &colorValue, sizeof (CColor));
}

//----------------------------------------------------------------------------------------------------
Property::Property (const CGraphicsTransform& transformValue)
: type (kTransformMatrix)
{
	value = std::malloc (sizeof (CGraphicsTransform));
	memcpy (value, &transformValue, sizeof (CGraphicsTransform));
}

//----------------------------------------------------------------------------------------------------
Property::Property (const Property& p)
: type (p.type)
, value (0)
{
	*this = p;
}

//----------------------------------------------------------------------------------------------------
Property::~Property ()
{
	if (value)
	{
		if (type == kObject)
			getObject ()->forget ();
		std::free (value);
	}
}

#if VSTGUI_RVALUE_REF_SUPPORT
//----------------------------------------------------------------------------------------------------
Property::Property (Property&& p) noexcept
{
	*this = std::move (p);
}

//----------------------------------------------------------------------------------------------------
Property& Property::operator=(Property&& p) noexcept
{
	type = p.type;
	value = p.value;
	p.value = nullptr;
	p.type = kNotFound;
	return *this;
}
#endif

//----------------------------------------------------------------------------------------------------
Property& Property::operator=(const Property& p)
{
	if (value)
	{
		if (type == kObject)
			getObject ()->forget ();
		std::free (value);
		value = 0;
	}
	type = p.type;
	if (p.value)
	{
		uint32_t valueSize;
		switch (type)
		{
			case kInteger: valueSize = sizeof (int64_t); break;
			case kFloat: valueSize = sizeof (double); break;
			case kObject: valueSize = sizeof (CBaseObject*); p.getObject ()->remember (); break;
			case kRect: valueSize = sizeof (CRect); break;
			case kPoint: valueSize = sizeof (CPoint); break;
			case kColor: valueSize = sizeof (CColor); break;
			case kTransformMatrix: valueSize = sizeof (CGraphicsTransform); break;
			case kNotFound: valueSize = 0; break;
		}
		if (valueSize)
		{
			value = std::malloc (valueSize);
			memcpy (value, p.value, valueSize);
		}
	}
	return *this;
}

//----------------------------------------------------------------------------------------------------
int32_t Property::getInteger () const
{
	return *static_cast<int32_t*> (value);
}

//----------------------------------------------------------------------------------------------------
double Property::getFloat () const
{
	return *static_cast<double*> (value);
}

//----------------------------------------------------------------------------------------------------
CBaseObject* Property::getObject () const
{
	return *static_cast<CBaseObject**> (value);
}

//----------------------------------------------------------------------------------------------------
const CRect& Property::getRect () const
{
	return *static_cast<CRect*> (value);
}

//----------------------------------------------------------------------------------------------------
const CPoint& Property::getPoint () const
{
	return *static_cast<CPoint*> (value);
}

//----------------------------------------------------------------------------------------------------
const CColor& Property::getColor () const
{
	return *static_cast<CColor*> (value);
}

//----------------------------------------------------------------------------------------------------
const CGraphicsTransform& Property::getTransform () const
{
	return *static_cast<CGraphicsTransform*> (value);
}

namespace Standard {
	static void registerStandardFilters (Factory& factory);
}

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
Factory& Factory::getInstance ()
{
	static Factory gInstance;
	static bool initialized = false;
	if (initialized == false)
	{
		Standard::registerStandardFilters (gInstance);
		initialized = true;
	}
	return gInstance;
}

//----------------------------------------------------------------------------------------------------
uint32_t Factory::getNumFilters () const
{
	return (uint32_t)filters.size ();
}

//----------------------------------------------------------------------------------------------------
IdStringPtr Factory::getFilterName (uint32_t index) const
{
	for (FilterMap::const_iterator it = filters.begin (), end = filters.end (); it != end; ++it, --index)
	{
		if (index == 0)
			return it->first.c_str ();
	}
	return 0;
}

//----------------------------------------------------------------------------------------------------
IFilter* Factory::createFilter (IdStringPtr name) const
{
	FilterMap::const_iterator it = filters.find (name);
	if (it != filters.end ())
		return (*it).second (name);
	return 0;
}

//----------------------------------------------------------------------------------------------------
bool Factory::registerFilter (IdStringPtr name, IFilter::CreateFunction createFunction)
{
	FilterMap::iterator it = filters.find (name);
	if (it != filters.end ())
		it->second = createFunction;
	else
		filters.insert (std::make_pair (name, createFunction));
	return true;
}

//----------------------------------------------------------------------------------------------------
bool Factory::unregisterFilter (IdStringPtr name, IFilter::CreateFunction createFunction)
{
	FilterMap::iterator it = filters.find (name);
	if (it == filters.end ())
		return false;
	filters.erase (it);
	return true;
}

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
FilterBase::FilterBase (UTF8StringPtr description)
: description (description ? description : "")
{
}

//----------------------------------------------------------------------------------------------------
UTF8StringPtr FilterBase::getDescription () const
{
	return description.c_str ();
}

//----------------------------------------------------------------------------------------------------
bool FilterBase::setProperty (IdStringPtr name, const Property& property)
{
	const_iterator it = find (name);
	if (it != end () && it->second.getType () == property.getType ())
	{
		(*this)[name] = property;
		return true;
	}
	return false;
}

//----------------------------------------------------------------------------------------------------
const Property& FilterBase::getProperty (IdStringPtr name) const
{
	const_iterator it = find (name);
	if (it != end ())
		return it->second;
	static Property notFound (Property::kNotFound);
	return notFound;
}

//----------------------------------------------------------------------------------------------------
uint32_t FilterBase::getNumProperties () const
{
	return (uint32_t)size ();
}

//----------------------------------------------------------------------------------------------------
IdStringPtr FilterBase::getPropertyName (uint32_t index) const
{
	for (const_iterator it = begin (); it != end (); it++, index--)
	{
		if (index == 0)
			return (*it).first.c_str ();
	}
	return 0;
}

//----------------------------------------------------------------------------------------------------
Property::Type FilterBase::getPropertyType (uint32_t index) const
{
	for (const_iterator it = begin (); it != end (); it++, index--)
	{
		if (index == 0)
			return (*it).second.getType ();
	}
	return Property::kNotFound;
}

//----------------------------------------------------------------------------------------------------
Property::Type FilterBase::getPropertyType (IdStringPtr name) const
{
	const_iterator it = find (name);
	if (it != end ())
	{
		return (*it).second.getType ();
	}
	return Property::kNotFound;
}

//----------------------------------------------------------------------------------------------------
bool FilterBase::registerProperty (IdStringPtr name, const Property& defaultProperty)
{
	return insert (std::make_pair (name, defaultProperty)).second;
}

//----------------------------------------------------------------------------------------------------
CBitmap* FilterBase::getInputBitmap () const
{
	const_iterator it = find (Standard::Property::kInputBitmap);
	if (it != end ())
	{
		CBaseObject* obj = (*it).second.getObject ();
		return obj ? dynamic_cast<CBitmap*>(obj) : 0;
	}
	return 0;
}

///@cond ignore
namespace Standard {

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
class BoxBlur : public FilterBase
{
public:
	static IFilter* CreateFunction (IdStringPtr _name)
	{
		return new BoxBlur ();
	}

private:
	BoxBlur ()
	: FilterBase ("A Box Blur Filter")
	{
		registerProperty (Property::kInputBitmap, BitmapFilter::Property (BitmapFilter::Property::kObject));
		registerProperty (Property::kRadius, BitmapFilter::Property ((int32_t)2));
	}

	bool run (bool replace) VSTGUI_OVERRIDE_VMETHOD
	{
		CBitmap* inputBitmap = getInputBitmap ();
		uint32_t radius = static_cast<uint32_t>(static_cast<double>(getProperty (Property::kRadius).getInteger ()) * inputBitmap->getPlatformBitmap ()->getScaleFactor ());
		if (inputBitmap == 0 || radius == UINT_MAX)
			return false;
		if (radius < 2)
		{
			if (replace)
				return true;
			return false; // TODO: We should just copy the input bitmap to the output bitmap
		}
		if (replace)
		{
			SharedPointer<CBitmapPixelAccess> inputAccessor = owned (CBitmapPixelAccess::create (inputBitmap));
			if (inputAccessor == 0)
				return false;
			run (*inputAccessor, *inputAccessor, radius);
			return registerProperty (Property::kOutputBitmap, BitmapFilter::Property (inputBitmap));
		}
		SharedPointer<CBitmap> outputBitmap = owned (new CBitmap (inputBitmap->getWidth (), inputBitmap->getHeight ()));
		if (outputBitmap)
		{
			SharedPointer<CBitmapPixelAccess> inputAccessor = owned (CBitmapPixelAccess::create (inputBitmap));
			SharedPointer<CBitmapPixelAccess> outputAccessor = owned (CBitmapPixelAccess::create (outputBitmap));
			if (inputAccessor == 0 || outputAccessor == 0)
				return false;

			run (*inputAccessor, *outputAccessor, radius);
			return registerProperty (Property::kOutputBitmap, BitmapFilter::Property (outputBitmap));
		}
		return false;
	}

	inline void calculate (CColor* colors, uint32_t numColors)
	{
		uint32_t lastColor = numColors - 1;
		int32_t red = colors[lastColor].red;
		int32_t green = colors[lastColor].green;
		int32_t blue = colors[lastColor].blue;
		int32_t alpha = colors[lastColor].alpha;
		for (int64_t i = (int64_t)numColors-2; i >= 0; i--)
		{
			red += colors[i].red;
			green += colors[i].green;
			blue += colors[i].blue;
			alpha += colors[i].alpha;
			colors[i+1] = colors[i];
		}
		colors[0].alpha = (uint8_t)(alpha / numColors);
		if (colors[0].alpha == 0)
		{
			colors[0].red = colors[0].green = colors[0].blue = 0;
		}
		else
		{
			colors[0].red = (uint8_t)(red / numColors);
			colors[0].green = (uint8_t)(green / numColors);
			colors[0].blue = (uint8_t)(blue / numColors);
		}
	}

	void run (CBitmapPixelAccess& inputAccessor, CBitmapPixelAccess& outputAccessor, uint32_t radius)
	{
		const uint32_t halfRadius = radius / 2;
		const uint32_t width = inputAccessor.getBitmapWidth ();
		const uint32_t height = inputAccessor.getBitmapHeight ();
		uint32_t x,y,x1,y1;
		CColor stackColors[20];
		CColor* nc;
		if (radius > 19)
			nc = new CColor[(size_t)radius];
		else
			nc = stackColors;
		for (y = 0; y < height; y++)
		{
			memset (nc, 0, sizeof(CColor) * (size_t)radius);
			for (x1 = 0; x1 < radius / 2; x1++)
			{
				inputAccessor.setPosition (x1, y);
				inputAccessor.getColor (nc[0]);
				calculate (nc, radius);
			}
			for (x = 0; x < width - halfRadius; x++, x1++)
			{
				inputAccessor.setPosition (x1, y);
				inputAccessor.getColor (nc[0]);
				calculate (nc, radius);
				outputAccessor.setPosition (x, y);
				outputAccessor.setColor (nc[0]);
			}
			for (;x < width; x++, x1++)
			{
				nc[0] = nc[1];
				calculate (nc, radius);
				outputAccessor.setPosition (x, y);
				outputAccessor.setColor (nc[0]);
			}
		}
		for (x = 0; x < width; x++)
		{
			memset (nc, 0, sizeof(CColor) * (size_t)radius);
			for (y1 = 0; y1 < radius / 2; y1++)
			{
				inputAccessor.setPosition (x, y1);
				inputAccessor.getColor (nc[0]);
				calculate (nc, radius);
			}
			for (y = 0; y < height - halfRadius; y++, y1++)
			{
				inputAccessor.setPosition (x, y1);
				inputAccessor.getColor (nc[0]);
				calculate (nc, radius);
				outputAccessor.setPosition (x, y);
				outputAccessor.setColor (nc[0]);
			}
			for (; y < height; y++, y1++)
			{
				nc[0] = nc[1];
				calculate (nc, radius);
				outputAccessor.setPosition (x, y);
				outputAccessor.setColor (nc[0]);
			}
		}
		if (radius > 19)
			delete [] nc;
	}

};

//----------------------------------------------------------------------------------------------------
class ScaleBase : public FilterBase
{
protected:
	ScaleBase (UTF8StringPtr description = "")
	: FilterBase (description)
	{
		registerProperty (Property::kInputBitmap, BitmapFilter::Property (BitmapFilter::Property::kObject));
		registerProperty (Property::kOutputRect, CRect (0, 0, 10, 10));
	}
	
	bool run (bool replace) VSTGUI_OVERRIDE_VMETHOD
	{
		if (replace)
			return false;
		CRect outSize = getProperty (Property::kOutputRect).getRect ();
		outSize.makeIntegral ();
		if (outSize.getWidth () <= 0 || outSize.getHeight () <= 0)
			return false;
		CBitmap* inputBitmap = getInputBitmap ();
		if (inputBitmap == 0)
			return false;
		SharedPointer<CBitmap> outputBitmap = owned (new CBitmap (outSize.getWidth (), outSize.getHeight ()));
		if (outputBitmap == 0)
			return false;
		
		SharedPointer<CBitmapPixelAccess> inputAccessor = owned (CBitmapPixelAccess::create (inputBitmap));
		SharedPointer<CBitmapPixelAccess> outputAccessor = owned (CBitmapPixelAccess::create (outputBitmap));
		if (inputAccessor == 0 || outputAccessor == 0)
			return false;
		process (*inputAccessor, *outputAccessor);
		return registerProperty (Property::kOutputBitmap, BitmapFilter::Property (outputBitmap));
	}
	
	virtual void process (CBitmapPixelAccess& originalBitmap, CBitmapPixelAccess& copyBitmap) = 0;
	
};

//----------------------------------------------------------------------------------------------------
class ScaleLinear : public ScaleBase
{
public:	
	static IFilter* CreateFunction (IdStringPtr _name)
	{
		return new ScaleLinear ();
	}

private:
	ScaleLinear () : ScaleBase ("A Linear Scale Filter") {}

	void process (CBitmapPixelAccess& originalBitmap, CBitmapPixelAccess& copyBitmap) VSTGUI_OVERRIDE_VMETHOD
	{
		originalBitmap.setPosition (0, 0);
		copyBitmap.setPosition (0, 0);
		
		int32_t origWidth = (int32_t)originalBitmap.getBitmapWidth ();
		int32_t origHeight = (int32_t)originalBitmap.getBitmapHeight ();
		int32_t newWidth = (int32_t)copyBitmap.getBitmapWidth ();
		int32_t newHeight = (int32_t)copyBitmap.getBitmapHeight ();
		
		float xRatio = (float)origWidth / (float)newWidth;
		float yRatio = (float)origHeight / (float)newHeight;

		uint8_t* origAddress = originalBitmap.getPlatformBitmapPixelAccess ()->getAddress ();
		uint8_t* copyAddress = copyBitmap.getPlatformBitmapPixelAccess ()->getAddress ();
		uint32_t origBytesPerRow = originalBitmap.getPlatformBitmapPixelAccess ()->getBytesPerRow ();
		uint32_t copyBytesPerRow = copyBitmap.getPlatformBitmapPixelAccess ()->getBytesPerRow ();

		CColor c;
		int32_t ix;
		int32_t iy = -1;
		int32_t* origPixel = 0;
		float origY = 0;
		float origX = 0;
		for (int32_t y = 0; y < newHeight; y++, origY += yRatio)
		{
			int32_t* copyPixel = (int32_t*)(copyAddress + y * copyBytesPerRow);
			if (iy != (int32_t)origY)
				iy = (int32_t)origY;
			ix = -1;
			origX = 0;
			for (int32_t x = 0; x < newWidth; x++, origX += xRatio, copyPixel++)
			{
				if (ix != (int32_t)origX || origPixel == 0)
				{
					ix = (int32_t)origX;
					origPixel = (int32_t*)(origAddress + iy * origBytesPerRow + ix * 4);
				}
				*copyPixel = *origPixel;
			}
		}
	}
};

//----------------------------------------------------------------------------------------------------
class ScaleBiliniear : public ScaleBase
{
public:
	static IFilter* CreateFunction (IdStringPtr _name)
	{
		return new ScaleBiliniear ();
	}

private:
	ScaleBiliniear () : ScaleBase ("A Biliniear Scale Filter") {}

	void process (CBitmapPixelAccess& originalBitmap, CBitmapPixelAccess& copyBitmap) VSTGUI_OVERRIDE_VMETHOD
	{
		originalBitmap.setPosition (0, 0);
		copyBitmap.setPosition (0, 0);

		int32_t origWidth = (int32_t)originalBitmap.getBitmapWidth ();
		int32_t origHeight = (int32_t)originalBitmap.getBitmapHeight ();
		int32_t newWidth = (int32_t)copyBitmap.getBitmapWidth ();
		int32_t newHeight = (int32_t)copyBitmap.getBitmapHeight ();

		float xRatio = ((float)(origWidth-1)) / (float)newWidth;
		float yRatio = ((float)(origHeight-1)) / (float)newHeight;
		float xDiff, yDiff, r, g, b, a;
		uint32_t x, y;
		CColor color[4];
		CColor result;

		for (int32_t i = 0; i < newHeight; i++)
		{
			y = (int32_t)(yRatio * i);
			yDiff = (yRatio * i) - y;

			for (int32_t j = 0; j < newWidth; j++, copyBitmap++)
			{
				x = (int32_t)(xRatio * j);
				xDiff = (xRatio * j) - x;
				originalBitmap.setPosition (x, y);
				originalBitmap.getColor (color[0]);
				originalBitmap.setPosition (x+1, y);
				originalBitmap.getColor (color[1]);
				originalBitmap.setPosition (x, y+1);
				originalBitmap.getColor (color[2]);
				originalBitmap.setPosition (x+1, y+1);
				originalBitmap.getColor (color[3]);
				r = color[0].red * (1.f - xDiff) * (1.f - yDiff) + color[1].red * xDiff * (1.f - yDiff)
				+ color[2].red * yDiff * (1.f - xDiff) + color[3].red * xDiff * yDiff;
				g = color[0].green * (1.f - xDiff) * (1.f - yDiff) + color[1].green * xDiff * (1.f - yDiff)
				+ color[2].green * yDiff * (1.f - xDiff) + color[3].green * xDiff * yDiff;
				b = color[0].blue * (1.f - xDiff) * (1.f - yDiff) + color[1].blue * xDiff * (1.f - yDiff)
				+ color[2].blue * yDiff * (1.f - xDiff) + color[3].blue * xDiff * yDiff;
				a = color[0].alpha * (1.f - xDiff) * (1.f - yDiff) + color[1].alpha * xDiff * (1.f - yDiff)
				+ color[2].alpha * yDiff * (1.f - xDiff) + color[3].alpha * xDiff * yDiff;
				result = CColor ((uint8_t)r, (uint8_t)g, (uint8_t)b, (uint8_t)a);
				copyBitmap.setColor (result);
			}
		}
	}
};

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
typedef void (*SimpleFilterProcessFunction) (CColor& color, FilterBase* self);

template<typename SimpleFilterProcessFunction>
class SimpleFilter : public FilterBase
{
protected:
	SimpleFilter (UTF8StringPtr description, SimpleFilterProcessFunction function)
	: FilterBase (description)
	, processFunction (function)
	{
		registerProperty (Property::kInputBitmap, BitmapFilter::Property (BitmapFilter::Property::kObject));
	}

	bool run (bool replace) VSTGUI_OVERRIDE_VMETHOD
	{
		SharedPointer<CBitmap> inputBitmap = getInputBitmap ();
		if (inputBitmap == 0)
			return false;
		SharedPointer<CBitmapPixelAccess> inputAccessor = owned (CBitmapPixelAccess::create (inputBitmap));
		if (inputAccessor == 0)
			return false;
		SharedPointer<CBitmap> outputBitmap;
		SharedPointer<CBitmapPixelAccess> outputAccessor;
		if (replace == false)
		{
			outputBitmap = owned (new CBitmap (inputBitmap->getWidth (), inputBitmap->getHeight ()));
			if (outputBitmap == 0)
				return false;
			outputAccessor = owned (CBitmapPixelAccess::create (outputBitmap));
			if (outputAccessor == 0)
				return false;
		}
		else
		{
			outputBitmap = inputBitmap;
			outputAccessor = inputAccessor;
		}
		run (*inputAccessor, *outputAccessor);
		return registerProperty (Property::kOutputBitmap, BitmapFilter::Property (outputBitmap));
	}

	void run (CBitmapPixelAccess& inputAccessor, CBitmapPixelAccess& outputAccessor)
	{
		inputAccessor.setPosition (0, 0);
		outputAccessor.setPosition (0, 0);
		CColor color;
		while (true)
		{
			inputAccessor.getColor (color);
			processFunction (color, this);
			outputAccessor.setColor (color);
			if (inputAccessor++ == false)
				break;
			if (&inputAccessor != &outputAccessor)
				outputAccessor++;
		}
	}

	SimpleFilterProcessFunction processFunction;
};

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
class SetColor : public SimpleFilter<SimpleFilterProcessFunction>
{
public:
	static IFilter* CreateFunction (IdStringPtr _name)
	{
		return new SetColor ();
	}

private:
	SetColor ()
	: SimpleFilter ("A Set Color Filter", processSetColor)
	{
		registerProperty (Property::kIgnoreAlphaColorValue, BitmapFilter::Property ((int32_t)1));
		registerProperty (Property::kInputColor, BitmapFilter::Property (kWhiteCColor));
	}

	static void processSetColor (CColor& color, FilterBase* obj)
	{
		SetColor* filter = static_cast<SetColor*> (obj);
		if (filter->ignoreAlpha)
			filter->inputColor.alpha = color.alpha;
		color = filter->inputColor;
	}

	bool ignoreAlpha;
	CColor inputColor;

	bool run (bool replace) VSTGUI_OVERRIDE_VMETHOD
	{
		inputColor = getProperty (Property::kInputColor).getColor ();
		ignoreAlpha = getProperty (Property::kIgnoreAlphaColorValue).getInteger () > 0;
		return SimpleFilter::run (replace);
	}
};

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
class Grayscale : public SimpleFilter<SimpleFilterProcessFunction>
{
public:
	static IFilter* CreateFunction (IdStringPtr name)
	{
		return new Grayscale ();
	}

private:
	Grayscale ()
	: SimpleFilter ("A Grayscale Filter", processGrayscale)
	{
	}

	static void processGrayscale (CColor& color, FilterBase* obj)
	{
		color.red = color.green = color.blue = color.getLuma ();
	}

};

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
class ReplaceColor : public SimpleFilter<SimpleFilterProcessFunction>
{
public:
	static IFilter* CreateFunction (IdStringPtr name)
	{
		return new ReplaceColor ();
	}

private:
	ReplaceColor ()
	: SimpleFilter ("A Replace Color Filter", processReplace)
	{
		registerProperty (Property::kInputColor, BitmapFilter::Property (kWhiteCColor));
		registerProperty (Property::kOutputColor, BitmapFilter::Property (kTransparentCColor));
	}

	static void processReplace (CColor& color, FilterBase* obj)
	{
		ReplaceColor* filter = static_cast<ReplaceColor*> (obj);
		if (color == filter->inputColor)
			color = filter->outputColor;
	}

	CColor inputColor;
	CColor outputColor;

	bool run (bool replace) VSTGUI_OVERRIDE_VMETHOD
	{
		inputColor = getProperty (Property::kInputColor).getColor ();
		outputColor = getProperty (Property::kOutputColor).getColor ();
		return SimpleFilter<SimpleFilterProcessFunction>::run (replace);
	}

};

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
void registerStandardFilters (Factory& factory)
{
	factory.registerFilter (kBoxBlur, BoxBlur::CreateFunction);
	factory.registerFilter (kSetColor, SetColor::CreateFunction);
	factory.registerFilter (kGrayscale, Grayscale::CreateFunction);
	factory.registerFilter (kReplaceColor, ReplaceColor::CreateFunction);
	factory.registerFilter (kScaleBilinear, ScaleBiliniear::CreateFunction);
	factory.registerFilter (kScaleLinear, ScaleLinear::CreateFunction);
}

} // namespace Standard

///@end cond

}} // namespaces