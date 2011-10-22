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
	value = malloc (sizeof (int32_t));
	memcpy (value, &intValue, sizeof (int32_t));
}

//----------------------------------------------------------------------------------------------------
Property::Property (double floatValue)
: type (kFloat)
{
	value = malloc (sizeof (double));
	memcpy (value, &floatValue, sizeof (double));
}

//----------------------------------------------------------------------------------------------------
Property::Property (CBaseObject* objectValue)
: type (kObject)
{
	value = malloc (sizeof (CBaseObject*));
	memcpy (value, &objectValue, sizeof (CBaseObject*));
	objectValue->remember ();
}

//----------------------------------------------------------------------------------------------------
Property::Property (const CRect& rectValue)
: type (kRect)
{
	value = malloc (sizeof (CRect));
	memcpy (value, &rectValue, sizeof (CRect));
}

//----------------------------------------------------------------------------------------------------
Property::Property (const CPoint& pointValue)
: type (kPoint)
{
	value = malloc (sizeof (CPoint));
	memcpy (value, &pointValue, sizeof (CPoint));
}

//----------------------------------------------------------------------------------------------------
Property::Property (const CColor& colorValue)
: type (kColor)
{
	value = malloc (sizeof (CColor));
	memcpy (value, &colorValue, sizeof (CColor));
}

//----------------------------------------------------------------------------------------------------
Property::Property (const CGraphicsTransform& transformValue)
: type (kTransformMatrix)
{
	value = malloc (sizeof (CGraphicsTransform));
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
		free (value);
	}
}

//----------------------------------------------------------------------------------------------------
Property& Property::operator=(const Property& p)
{
	if (value)
	{
		if (type == kObject)
			getObject ()->forget ();
		free (value);
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
			value = malloc (valueSize);
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
	if (getNumFilters () > index)
		return filters[index].first.c_str ();
	return 0;
}

//----------------------------------------------------------------------------------------------------
IFilter* Factory::createFilter (IdStringPtr name) const
{
	for (std::vector<std::pair<std::string, IFilter::CreateFunction> >::const_iterator it = filters.begin (); it != filters.end (); it++)
	{
		if ((*it).first == name)
			return (*it).second (name);
	}
	return 0;
}

//----------------------------------------------------------------------------------------------------
bool Factory::registerFilter (IdStringPtr name, IFilter::CreateFunction createFunction)
{
	filters.push_back (std::make_pair (name, createFunction));
	return true;
}

//----------------------------------------------------------------------------------------------------
bool Factory::unregisterFilter (IdStringPtr name, IFilter::CreateFunction createFunction)
{
	for (std::vector<std::pair<std::string, IFilter::CreateFunction> >::iterator it = filters.begin (); it != filters.end (); it++)
	{
		if ((*it).first == name)
		{
			filters.erase (it);
			return true;
		}
	}
	return false;
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
	BoxBlur ()
	: FilterBase ("A Box Blur Filter")
	{
		registerProperty (Property::kInputBitmap, BitmapFilter::Property (BitmapFilter::Property::kObject));
		registerProperty (Property::kRadius, BitmapFilter::Property ((int32_t)2));
	}

	static IFilter* CreateFunction (IdStringPtr _name)
	{
		return new BoxBlur ();
	}

	bool run (bool replace)
	{
		CBitmap* inputBitmap = getInputBitmap ();
		uint32_t radius = (uint32_t)getProperty (Property::kRadius).getInteger ();
		if (inputBitmap == 0 || radius < 2 || radius == ULONG_MAX)
			return false;
		if (replace)
		{
			OwningPointer<CBitmapPixelAccess> inputAccessor = CBitmapPixelAccess::create (inputBitmap);
			if (inputAccessor == 0)
				return false;
			run (*inputAccessor, *inputAccessor, radius);
			return registerProperty (Property::kOutputBitmap, BitmapFilter::Property (inputBitmap));
		}
		OwningPointer<CBitmap> outputBitmap = new CBitmap (inputBitmap->getWidth (), inputBitmap->getHeight ());
		if (outputBitmap)
		{
			OwningPointer<CBitmapPixelAccess> inputAccessor = CBitmapPixelAccess::create (inputBitmap);
			OwningPointer<CBitmapPixelAccess> outputAccessor = CBitmapPixelAccess::create (outputBitmap);
			if (inputAccessor == 0 || outputAccessor == 0)
				return false;

			run (*inputAccessor, *outputAccessor, radius);
			return registerProperty (Property::kOutputBitmap, BitmapFilter::Property (outputBitmap));
		}
		return false;
	}

	void run (CBitmapPixelAccess& inputAccessor, CBitmapPixelAccess& outputAccessor, uint32_t radius)
	{
		uint32_t x,y,x1,y1;
		uint32_t width = inputAccessor.getBitmapWidth ();
		uint32_t height = inputAccessor.getBitmapHeight ();
		CColor* nc = new CColor[(size_t)radius];
		for (y = 0; y < height; y++)
		{
			for (uint32_t i = 1; i < radius; i++)
				nc[i] = kTransparentCColor;
			for (x1 = 0; x1 < radius / 2; x1++)
			{
				inputAccessor.setPosition (x1, y);
				inputAccessor.getColor (nc[0]);
				calculate(nc[0], nc, radius);
			}
			for (x = 0; x < width; x++, x1++)
			{
				if (inputAccessor.setPosition (x1, y))
					inputAccessor.getColor (nc[0]);
				else
					nc[0] = kTransparentCColor;
				calculate (nc[0], nc, radius);
				outputAccessor.setPosition (x, y);
				outputAccessor.setColor (nc[0]);
			}
		}
		for (x = 0; x < width; x++)
		{
			for (uint32_t i = 1; i < radius; i++)
				nc[i] = kTransparentCColor;
			for (y1 = 0; y1 < radius / 2; y1++)
			{
				inputAccessor.setPosition (x, y1);
				inputAccessor.getColor (nc[0]);
				calculate(nc[0], nc, radius);
			}
			for (y = 0; y < height; y++, y1++)
			{
				if (inputAccessor.setPosition (x, y1))
					inputAccessor.getColor (nc[0]);
				else
					nc[0] = kTransparentCColor;
				calculate (nc[0], nc, radius);
				outputAccessor.setPosition (x, y);
				outputAccessor.setColor (nc[0]);
			}
		}
		delete [] nc;
	}

	void calculate (CColor& color, CColor* colors, uint32_t numColors)
	{
		int32_t red = 0;
		int32_t green = 0;
		int32_t blue = 0;
		int32_t alpha = 0;
		for (int64_t i = (int64_t)numColors-1; i >= 0; i--)
		{
			red += colors[i].red;
			green += colors[i].green;
			blue += colors[i].blue;
			alpha += colors[i].alpha;
			if (i+1 < numColors)
				colors[i+1] = colors[i];
		}
		red /= numColors;
		green /= numColors;
		blue /= numColors;
		alpha /= numColors;
		color.red = (uint8_t)red;
		color.green = (uint8_t)green;
		color.blue = (uint8_t)blue;
		color.alpha = (uint8_t)alpha;
	}
};

//----------------------------------------------------------------------------------------------------
class ScaleBiliniear : public FilterBase
{
public:
	ScaleBiliniear ()
	: FilterBase ("")
	{
		registerProperty (Property::kInputBitmap, BitmapFilter::Property (BitmapFilter::Property::kObject));
		registerProperty (Property::kOutputRect, CRect (0, 0, 10, 10));
	}
	
	static IFilter* CreateFunction (IdStringPtr _name)
	{
		return new ScaleBiliniear ();
	}
	
	bool run (bool replace)
	{
		if (replace)
			return false;
		CRect outSize = getProperty (Property::kOutputRect).getRect ();
		if (outSize.getWidth () <= 0 || outSize.getHeight () <= 0)
			return false;
		CBitmap* inputBitmap = getInputBitmap ();
		if (inputBitmap == 0)
			return false;
		OwningPointer<CBitmap> outputBitmap = new CBitmap (outSize.getWidth (), outSize.getHeight ());
		if (outputBitmap == 0)
			return false;

		OwningPointer<CBitmapPixelAccess> inputAccessor = CBitmapPixelAccess::create (inputBitmap);		
		OwningPointer<CBitmapPixelAccess> outputAccessor = CBitmapPixelAccess::create (outputBitmap);
		if (inputAccessor == 0 || outputAccessor == 0)
			return false;
		process (*inputAccessor, *outputAccessor);
		return registerProperty (Property::kOutputBitmap, BitmapFilter::Property (outputBitmap));
	}

	void process (CBitmapPixelAccess& originalBitmap, CBitmapPixelAccess& copyBitmap)
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
class SimpleFilter : public FilterBase
{
public:
	typedef void (*ProcessFunction) (CColor& color, FilterBase* self);
	
	SimpleFilter (UTF8StringPtr description, ProcessFunction function)
	: FilterBase (description)
	, processFunction (function)
	{
		registerProperty (Property::kInputBitmap, BitmapFilter::Property (BitmapFilter::Property::kObject));
	}
	
	bool run (bool replace)
	{
		CBitmap* inputBitmap = getInputBitmap ();
		if (inputBitmap == 0)
			return false;
		OwningPointer<CBitmapPixelAccess> inputAccessor = CBitmapPixelAccess::create (inputBitmap);
		if (inputAccessor == 0)
			return false;
		SharedPointer<CBitmap> outputBitmap = inputBitmap;
		SharedPointer<CBitmapPixelAccess> outputAccessor = inputAccessor;
		if (replace == false)
		{
			outputBitmap = new CBitmap (inputBitmap->getWidth (), inputBitmap->getHeight ());
			if (outputBitmap == 0)
				return false;
			outputBitmap->forget (); // ownership by shared pointer
			outputAccessor = CBitmapPixelAccess::create (outputBitmap);
			if (outputAccessor == 0)
				return false;
			outputAccessor->forget (); // ownership by shared pointer
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

	ProcessFunction processFunction;
};

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
class SetColor : public SimpleFilter
{
public:
	SetColor ()
	: SimpleFilter ("", processSetColor)
	{
		registerProperty (Property::kIgnoreAlphaColorValue, BitmapFilter::Property ((int32_t)1));
		registerProperty (Property::kInputColor, BitmapFilter::Property (kWhiteCColor));
	}
	
	static IFilter* CreateFunction (IdStringPtr _name)
	{
		return new SetColor ();
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

	bool run (bool replace)
	{
		inputColor = getProperty (Property::kInputColor).getColor ();
		ignoreAlpha = getProperty (Property::kIgnoreAlphaColorValue).getInteger () > 0;
		return SimpleFilter::run (replace);
	}
};

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
class Grayscale : public SimpleFilter
{
public:
	Grayscale ()
	: SimpleFilter ("", processGrayscale)
	{
	}
	
	static IFilter* CreateFunction (IdStringPtr name)
	{
		return new Grayscale ();
	}

	static void processGrayscale (CColor& color, FilterBase* obj)
	{
		color.red = color.green = color.blue = color.getLuma ();
	}
	
};

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
class ReplaceColor : public SimpleFilter
{
public:
	ReplaceColor ()
	: SimpleFilter ("", processReplace)
	{
		registerProperty (Property::kInputColor, BitmapFilter::Property (kWhiteCColor));
		registerProperty (Property::kOutputColor, BitmapFilter::Property (kTransparentCColor));
	}
	
	static IFilter* CreateFunction (IdStringPtr name)
	{
		return new ReplaceColor ();
	}
	
	static void processReplace (CColor& color, FilterBase* obj)
	{
		ReplaceColor* filter = static_cast<ReplaceColor*> (obj);
		if (color == filter->inputColor)
			color = filter->outputColor;
	}
	
	CColor inputColor;
	CColor outputColor;

	bool run (bool replace)
	{
		inputColor = getProperty (Property::kInputColor).getColor ();
		outputColor = getProperty (Property::kOutputColor).getColor ();
		return SimpleFilter::run (replace);
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
}

} // namespace Standard

///@end cond

}} // namespaces
