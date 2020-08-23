// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "cbitmapfilter.h"
#include "cbitmap.h"
#include "platform/iplatformbitmap.h"
#include "ccolor.h"
#include "cgraphicspath.h"
#include "cgraphicstransform.h"
#include "malloc.h"
#include <cassert>
#include <algorithm>
#include <memory>

namespace VSTGUI {

namespace BitmapFilter {

//----------------------------------------------------------------------------------------------------
template<typename T> void Property::assign (T toAssign)
{
	value = std::malloc (sizeof (toAssign));
	memcpy (value, &toAssign, sizeof (toAssign));
}

//----------------------------------------------------------------------------------------------------
Property::Property (Type type)
: type (type)
, value (nullptr)
{
}

//----------------------------------------------------------------------------------------------------
Property::Property (int32_t intValue)
: type (kInteger)
{
	assign (intValue);
}

//----------------------------------------------------------------------------------------------------
Property::Property (double floatValue)
: type (kFloat)
{
	assign (floatValue);
}

//----------------------------------------------------------------------------------------------------
Property::Property (IReference* objectValue)
: type (kObject)
{
	value = static_cast<void*> (objectValue);
	objectValue->remember ();
}

//----------------------------------------------------------------------------------------------------
Property::Property (const CRect& rectValue)
: type (kRect)
{
	assign (rectValue);
}

//----------------------------------------------------------------------------------------------------
Property::Property (const CPoint& pointValue)
: type (kPoint)
{
	assign (pointValue);
}

//----------------------------------------------------------------------------------------------------
Property::Property (const CColor& colorValue)
: type (kColor)
{
	assign (colorValue);
}

//----------------------------------------------------------------------------------------------------
Property::Property (const CGraphicsTransform& transformValue)
: type (kTransformMatrix)
{
	assign (transformValue);
}

//----------------------------------------------------------------------------------------------------
Property::Property (const Property& p)
: type (p.type)
, value (nullptr)
{
	*this = p;
}

//----------------------------------------------------------------------------------------------------
Property::~Property () noexcept
{
	if (value)
	{
		if (type == kObject)
			getObject ()->forget ();
		else
			std::free (value);
	}
}

//----------------------------------------------------------------------------------------------------
Property::Property (Property&& p) noexcept
: value (nullptr)
{
	*this = std::move (p);
}

//----------------------------------------------------------------------------------------------------
Property& Property::operator=(Property&& p) noexcept
{
	if (value)
	{
		if (type == kObject)
			getObject ()->forget ();
		else
			std::free (value);
	}
	type = p.type;
	value = p.value;
	p.value = nullptr;
	p.type = kUnknown;
	return *this;
}

//----------------------------------------------------------------------------------------------------
Property& Property::operator=(const Property& p)
{
	if (value)
	{
		if (type == kObject)
			getObject ()->forget ();
		else
			std::free (value);
		value = nullptr;
	}
	type = p.type;
	if (p.value)
	{
		uint32_t valueSize = 0u;
		switch (type)
		{
			case kInteger: valueSize = sizeof (int32_t); break;
			case kFloat: valueSize = sizeof (double); break;
			case kObject: value = p.value; p.getObject ()->remember (); break;
			case kRect: valueSize = sizeof (CRect); break;
			case kPoint: valueSize = sizeof (CPoint); break;
			case kColor: valueSize = sizeof (CColor); break;
			case kTransformMatrix: valueSize = sizeof (CGraphicsTransform); break;
			case kUnknown: valueSize = 0u; break;
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
	vstgui_assert (type == kInteger);
	return *static_cast<int32_t*> (value);
}

//----------------------------------------------------------------------------------------------------
double Property::getFloat () const
{
	vstgui_assert (type == kFloat);
	return *static_cast<double*> (value);
}

//----------------------------------------------------------------------------------------------------
IReference* Property::getObject () const
{
	vstgui_assert (type == kObject);
	return static_cast<IReference*> (value);
}

//----------------------------------------------------------------------------------------------------
const CRect& Property::getRect () const
{
	vstgui_assert (type == kRect);
	return *static_cast<CRect*> (value);
}

//----------------------------------------------------------------------------------------------------
const CPoint& Property::getPoint () const
{
	vstgui_assert (type == kPoint);
	return *static_cast<CPoint*> (value);
}

//----------------------------------------------------------------------------------------------------
const CColor& Property::getColor () const
{
	vstgui_assert (type == kColor);
	return *static_cast<CColor*> (value);
}

//----------------------------------------------------------------------------------------------------
const CGraphicsTransform& Property::getTransform () const
{
	vstgui_assert (type == kTransformMatrix);
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
	for (const auto& filter : filters)
	{
		if (index == 0)
			return filter.first.c_str ();
		--index;
	}
	return nullptr;
}

//----------------------------------------------------------------------------------------------------
IFilter* Factory::createFilter (IdStringPtr name) const
{
	FilterMap::const_iterator it = filters.find (name);
	if (it != filters.end ())
		return (*it).second (name);
	return nullptr;
}

//----------------------------------------------------------------------------------------------------
bool Factory::registerFilter (IdStringPtr name, IFilter::CreateFunction createFunction)
{
	FilterMap::iterator it = filters.find (name);
	if (it != filters.end ())
		it->second = createFunction;
	else
		filters.emplace (name, createFunction);
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
	auto it = properties.find (name);
	if (it != properties.end () && it->second.getType () == property.getType ())
	{
		properties[name] = property;
		return true;
	}
	return false;
}

//----------------------------------------------------------------------------------------------------
bool FilterBase::setProperty (IdStringPtr name, Property&& property)
{
	auto it = properties.find (name);
	if (it != properties.end () && it->second.getType () == property.getType ())
	{
		properties[name] = std::move (property);
		return true;
	}
	return false;
}

//----------------------------------------------------------------------------------------------------
const Property& FilterBase::getProperty (IdStringPtr name) const
{
	auto it = properties.find (name);
	if (it != properties.end ())
		return it->second;
	static Property notFound (Property::kUnknown);
	return notFound;
}

//----------------------------------------------------------------------------------------------------
uint32_t FilterBase::getNumProperties () const
{
	return static_cast<uint32_t> (properties.size ());
}

//----------------------------------------------------------------------------------------------------
IdStringPtr FilterBase::getPropertyName (uint32_t index) const
{
	for (const auto & it : properties)
	{
		if (index == 0)
			return it.first.c_str ();
		index--;
	}
	return nullptr;
}

//----------------------------------------------------------------------------------------------------
Property::Type FilterBase::getPropertyType (uint32_t index) const
{
	for (const auto & it : properties)
	{
		if (index == 0)
			return it.second.getType ();
		index--;
	}
	return Property::kUnknown;
}

//----------------------------------------------------------------------------------------------------
Property::Type FilterBase::getPropertyType (IdStringPtr name) const
{
	auto it = properties.find (name);
	if (it != properties.end ())
	{
		return (*it).second.getType ();
	}
	return Property::kUnknown;
}

//----------------------------------------------------------------------------------------------------
bool FilterBase::registerProperty (IdStringPtr name, const Property& defaultProperty)
{
	return properties.emplace (name, defaultProperty).second;
}

//----------------------------------------------------------------------------------------------------
CBitmap* FilterBase::getInputBitmap () const
{
	auto it = properties.find (Standard::Property::kInputBitmap);
	if (it != properties.end ())
	{
		auto obj = (*it).second.getObject ();
		return obj ? dynamic_cast<CBitmap*>(obj) : nullptr;
	}
	return nullptr;
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
		registerProperty (Property::kAlphaChannelOnly, BitmapFilter::Property ((int32_t)0));
	}

	bool run (bool replace) override
	{
		CBitmap* inputBitmap = getInputBitmap ();
		if (inputBitmap == nullptr)
			return false;
		const auto& radiusProp = getProperty (Property::kRadius);
		if (radiusProp.getType () != BitmapFilter::Property::kInteger)
			return false;
		uint32_t radius =
		    static_cast<uint32_t> (static_cast<double> (radiusProp.getInteger ()) *
		                           inputBitmap->getPlatformBitmap ()->getScaleFactor ());
		if (radius == UINT_MAX)
			return false;
		if (radius < 2)
		{
			if (replace)
				return true;
			return false; // TODO: We should just copy the input bitmap to the output bitmap
		}
		const auto& alphaChannelOnlyProp = getProperty (Property::kAlphaChannelOnly);
		if (alphaChannelOnlyProp.getType () != BitmapFilter::Property::kInteger)
			return false;
		bool alphaChannelOnly = alphaChannelOnlyProp.getInteger () > 0 ? true : false;
		if (replace)
		{
			SharedPointer<CBitmapPixelAccess> inputAccessor = owned (CBitmapPixelAccess::create (inputBitmap));
			if (inputAccessor == nullptr)
				return false;
			run (*inputAccessor, *inputAccessor, radius, alphaChannelOnly);
			return registerProperty (Property::kOutputBitmap, BitmapFilter::Property (inputBitmap));
		}
		SharedPointer<CBitmap> outputBitmap = owned (new CBitmap (inputBitmap->getWidth (), inputBitmap->getHeight ()));
		if (outputBitmap)
		{
			SharedPointer<CBitmapPixelAccess> inputAccessor = owned (CBitmapPixelAccess::create (inputBitmap));
			SharedPointer<CBitmapPixelAccess> outputAccessor = owned (CBitmapPixelAccess::create (outputBitmap));
			if (inputAccessor == nullptr || outputAccessor == nullptr)
				return false;

			run (*inputAccessor, *outputAccessor, radius, alphaChannelOnly);
			return registerProperty (Property::kOutputBitmap, BitmapFilter::Property (outputBitmap));
		}
		return false;
	}

	void run (CBitmapPixelAccess& inputAccessor, CBitmapPixelAccess& outputAccessor, uint32_t radius, bool alphaChannelOnly)
	{
		auto inputPbpa = inputAccessor.getPlatformBitmapPixelAccess ();
		auto outputPbpa = outputAccessor.getPlatformBitmapPixelAccess ();
		auto inputAddressPtr = inputPbpa->getAddress ();
		auto outputAddressPtr = outputPbpa->getAddress ();
		auto width = inputPbpa->getBytesPerRow () / 4;
		auto height = inputAccessor.getBitmapHeight ();
		if (alphaChannelOnly)
		{
			switch (inputPbpa->getPixelFormat ())
			{
				case IPlatformBitmapPixelAccess::kARGB:
				case IPlatformBitmapPixelAccess::kABGR:
				{
					algo<true, false, false, false> (inputAddressPtr, outputAddressPtr, width, height, static_cast<int32_t> (radius / 2));
					break;
				}
				case IPlatformBitmapPixelAccess::kRGBA:
				case IPlatformBitmapPixelAccess::kBGRA:
				{
					algo<false, false, false, true> (inputAddressPtr, outputAddressPtr, width, height, static_cast<int32_t> (radius / 2));
					break;
				}
			}
		
		}
		else
		{
			algo<true, true, true, true> (inputAddressPtr, outputAddressPtr, width, height, static_cast<int32_t> (radius / 2));
		}
	}

	Buffer<uint8_t> pc0;
	Buffer<uint8_t> pc1;
	Buffer<uint8_t> pc2;
	Buffer<uint8_t> pc3;
	Buffer<int32_t> vMin;
	Buffer<int32_t> vMax;
	Buffer<uint8_t> dv;

	template<bool plane0, bool plane1, bool plane2, bool plane3>
	void algo (uint8_t* inPixel, uint8_t* outPixel, int32_t width, int32_t height, int32_t radius)
	{
		vstgui_assert (radius > 0);

		constexpr int32_t pos0 = 0;
		constexpr int32_t pos1 = 1;
		constexpr int32_t pos2 = 2;
		constexpr int32_t pos3 = 3;
		constexpr int32_t numComponents = 4;

		int32_t wm = width - 1;
		int32_t hm = height - 1;
		int32_t areaSize = width * height;
		int32_t div = radius + radius + 1;
		
		if (plane0)
			pc0.allocate (areaSize);
		if (plane1)
			pc1.allocate (areaSize);
		if (plane2)
			pc2.allocate (areaSize);
		if (plane3)
			pc3.allocate (areaSize);
		vMin.allocate (std::max (width, height));
		vMax.allocate (std::max (width, height));
		dv.allocate (256 * div);

		for (auto i = 0u; i < dv.size (); ++i)
			dv[i] = (i / div);

		int32_t sum0, sum1, sum2, sum3;
		for (auto y = 0, yw = 0, yi = 0; y < height; ++y, yw += width)
		{
			sum0 = sum1 = sum2 = sum3 = 0;
			for (auto i = -radius; i <= radius; i++)
			{
				auto p = (yi + std::min (wm, std::max (i, 0))) * numComponents;
				if (plane0)
					sum0 += inPixel[p + pos0];
				if (plane1)
					sum1 += inPixel[p + pos1];
				if (plane2)
					sum2 += inPixel[p + pos2];
				if (plane3)
					sum3 += inPixel[p + pos3];
			}
			if (y == 0)
			{
				for (auto x = 0; x < width; ++x, ++yi)
				{
					if (plane0)
						pc0[yi] = dv[sum0];
					if (plane1)
						pc1[yi] = dv[sum1];
					if (plane2)
						pc2[yi] = dv[sum2];
					if (plane3)
						pc3[yi] = dv[sum3];
					vMin[x] = std::min (x + radius + 1, wm);
					vMax[x] = std::max (x - radius, 0);
					auto p1 = (yw + vMin[x]) * numComponents;
					auto p2 = (yw + vMax[x]) * numComponents;
					if (plane0)
						sum0 += inPixel[p1 + pos0] - inPixel[p2 + pos0];
					if (plane1)
						sum1 += inPixel[p1 + pos1] - inPixel[p2 + pos1];
					if (plane2)
						sum2 += inPixel[p1 + pos2] - inPixel[p2 + pos2];
					if (plane3)
						sum3 += inPixel[p1 + pos3] - inPixel[p2 + pos3];
				}
			}
			else
			{
				for (auto x = 0; x < width; ++x, ++yi)
				{
					if (plane0)
						pc0[yi] = dv[sum0];
					if (plane1)
						pc1[yi] = dv[sum1];
					if (plane2)
						pc2[yi] = dv[sum2];
					if (plane3)
						pc3[yi] = dv[sum3];
					auto p1 = (yw + vMin[x]) * numComponents;
					auto p2 = (yw + vMax[x]) * numComponents;
					if (plane0)
						sum0 += inPixel[p1 + pos0] - inPixel[p2 + pos0];
					if (plane1)
						sum1 += inPixel[p1 + pos1] - inPixel[p2 + pos1];
					if (plane2)
						sum2 += inPixel[p1 + pos2] - inPixel[p2 + pos2];
					if (plane3)
						sum3 += inPixel[p1 + pos3] - inPixel[p2 + pos3];
				}
			}
		}

		sum0 = sum1 = sum2 = sum3 = 0;
		for (auto i = -radius, yp = -radius * width; i <= radius; ++i, yp += width)
		{
			auto yi = std::max (0, yp);
			if (plane0)
				sum0 += pc0[yi];
			if (plane1)
				sum1 += pc1[yi];
			if (plane2)
				sum2 += pc2[yi];
			if (plane3)
				sum3 += pc3[yi];
		}
		for (auto y = 0, yi = 0; y < height; ++y, yi += width)
		{
			auto pos = yi * numComponents;
			if (plane0)
				outPixel[pos + pos0] = dv[sum0];
			if (plane1)
				outPixel[pos + pos1] = dv[sum1];
			if (plane2)
				outPixel[pos + pos2] = dv[sum2];
			if (plane3)
				outPixel[pos + pos3] = dv[sum3];
			vMin[y] = std::min (y + radius + 1, hm) * width;
			vMax[y] = std::max (y - radius, 0) * width;
			auto p1 = vMin[y];
			auto p2 = vMax[y];
			if (plane0)
				sum0 += pc0[p1] - pc0[p2];
			if (plane1)
				sum1 += pc1[p1] - pc1[p2];
			if (plane2)
				sum2 += pc2[p1] - pc2[p2];
			if (plane3)
				sum3 += pc3[p1] - pc3[p2];
		}

		for (auto x = 1; x < width; ++x)
		{
			sum0 = sum1 = sum2 = sum3 = 0;
			for (auto i = -radius, yp = -radius * width; i <= radius; ++i, yp += width)
			{
				auto yi = std::max (0, yp) + x;
				if (plane0)
					sum0 += pc0[yi];
				if (plane1)
					sum1 += pc1[yi];
				if (plane2)
					sum2 += pc2[yi];
				if (plane3)
					sum3 += pc3[yi];
			}
			for (auto y = 0, yi = x; y < height; ++y, yi += width)
			{
				auto pos = yi * numComponents;
				if (plane0)
					outPixel[pos + pos0] = dv[sum0];
				if (plane1)
					outPixel[pos + pos1] = dv[sum1];
				if (plane2)
					outPixel[pos + pos2] = dv[sum2];
				if (plane3)
					outPixel[pos + pos3] = dv[sum3];
				auto p1 = x + vMin[y];
				auto p2 = x + vMax[y];
				if (plane0)
					sum0 += pc0[p1] - pc0[p2];
				if (plane1)
					sum1 += pc1[p1] - pc1[p2];
				if (plane2)
					sum2 += pc2[p1] - pc2[p2];
				if (plane3)
					sum3 += pc3[p1] - pc3[p2];
			}
		}
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
	
	bool run (bool replace) override
	{
		if (replace)
			return false;
		const auto& outSizeProp = getProperty (Property::kOutputRect);
		if (outSizeProp.getType () != BitmapFilter::Property::kRect)
			return false;
		CRect outSize = outSizeProp.getRect ();
		outSize.makeIntegral ();
		if (outSize.getWidth () <= 0 || outSize.getHeight () <= 0)
			return false;
		CBitmap* inputBitmap = getInputBitmap ();
		if (inputBitmap == nullptr)
			return false;
		SharedPointer<CBitmap> outputBitmap = owned (new CBitmap (outSize.getWidth (), outSize.getHeight ()));
		if (outputBitmap == nullptr)
			return false;
		
		SharedPointer<CBitmapPixelAccess> inputAccessor = owned (CBitmapPixelAccess::create (inputBitmap));
		SharedPointer<CBitmapPixelAccess> outputAccessor = owned (CBitmapPixelAccess::create (outputBitmap));
		if (inputAccessor == nullptr || outputAccessor == nullptr)
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

	void process (CBitmapPixelAccess& originalBitmap, CBitmapPixelAccess& copyBitmap) override
	{
		originalBitmap.setPosition (0, 0);
		copyBitmap.setPosition (0, 0);
		
		uint32_t origWidth = (uint32_t)originalBitmap.getBitmapWidth ();
		uint32_t origHeight = (uint32_t)originalBitmap.getBitmapHeight ();
		uint32_t newWidth = (uint32_t)copyBitmap.getBitmapWidth ();
		uint32_t newHeight = (uint32_t)copyBitmap.getBitmapHeight ();
		
		float xRatio = (float)origWidth / (float)newWidth;
		float yRatio = (float)origHeight / (float)newHeight;

		uint8_t* origAddress = originalBitmap.getPlatformBitmapPixelAccess ()->getAddress ();
		uint8_t* copyAddress = copyBitmap.getPlatformBitmapPixelAccess ()->getAddress ();
		uint32_t origBytesPerRow = originalBitmap.getPlatformBitmapPixelAccess ()->getBytesPerRow ();
		uint32_t copyBytesPerRow = copyBitmap.getPlatformBitmapPixelAccess ()->getBytesPerRow ();

		int32_t ix;
		int32_t iy = -1;
		int32_t* origPixel = nullptr;
		float origY = 0;
		float origX = 0;
		for (uint32_t y = 0; y < newHeight; y++, origY += yRatio)
		{
			int32_t* copyPixel = (int32_t*)(copyAddress + y * copyBytesPerRow);
			if (iy != (int32_t)origY)
				iy = (int32_t)origY;
			ix = -1;
			origX = 0;
			for (uint32_t x = 0; x < newWidth; x++, origX += xRatio, copyPixel++)
			{
				if (ix != (int32_t)origX || origPixel == nullptr)
				{
					ix = (int32_t)origX;
					vstgui_assert (iy >= 0);
					origPixel = (int32_t*)(origAddress + static_cast<uint32_t> (iy) * origBytesPerRow + ix * 4);
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

	void process (CBitmapPixelAccess& originalBitmap, CBitmapPixelAccess& copyBitmap) override
	{
		originalBitmap.setPosition (0, 0);
		copyBitmap.setPosition (0, 0);

		uint32_t origWidth = (uint32_t)originalBitmap.getBitmapWidth ();
		uint32_t origHeight = (uint32_t)originalBitmap.getBitmapHeight ();
		uint32_t newWidth = (uint32_t)copyBitmap.getBitmapWidth ();
		uint32_t newHeight = (uint32_t)copyBitmap.getBitmapHeight ();

		float xRatio = ((float)(origWidth-1)) / (float)newWidth;
		float yRatio = ((float)(origHeight-1)) / (float)newHeight;
		float xDiff, yDiff, r, g, b, a;
		uint32_t x, y;
		CColor color[4];
		CColor result;

		for (uint32_t i = 0; i < newHeight; i++)
		{
			y = static_cast<uint32_t> (yRatio * i);
			yDiff = (yRatio * i) - y;

			for (uint32_t j = 0; j < newWidth; j++, ++copyBitmap)
			{
				x = static_cast<uint32_t> (xRatio * j);
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
using SimpleFilterProcessFunction = void (*) (CColor& color, FilterBase* self);

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

	bool run (bool replace) override
	{
		SharedPointer<CBitmap> inputBitmap = getInputBitmap ();
		if (inputBitmap == nullptr)
			return false;
		SharedPointer<CBitmapPixelAccess> inputAccessor = owned (CBitmapPixelAccess::create (inputBitmap));
		if (inputAccessor == nullptr)
			return false;
		SharedPointer<CBitmap> outputBitmap;
		SharedPointer<CBitmapPixelAccess> outputAccessor;
		if (replace == false)
		{
			outputBitmap = owned (new CBitmap (inputBitmap->getWidth (), inputBitmap->getHeight ()));
			if (outputBitmap == nullptr)
				return false;
			outputAccessor = owned (CBitmapPixelAccess::create (outputBitmap));
			if (outputAccessor == nullptr)
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
		if (&inputAccessor == &outputAccessor)
		{
			do
			{
				inputAccessor.getColor (color);
				processFunction (color, this);
				outputAccessor.setColor (color);
			}
			while (++inputAccessor);
		}
		else
		{
			do
			{
				inputAccessor.getColor (color);
				processFunction (color, this);
				outputAccessor.setColor (color);
				++outputAccessor;
			}
			while (++inputAccessor);
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
	: SimpleFilter<SimpleFilterProcessFunction> ("A Set Color Filter", processSetColor)
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

	bool run (bool replace) override
	{
		const auto& inputColorProp = getProperty (Property::kInputColor);
		const auto& ignoreAlphaProp = getProperty (Property::kIgnoreAlphaColorValue);
		if (inputColorProp.getType () != BitmapFilter::Property::kColor ||
		    ignoreAlphaProp.getType () != BitmapFilter::Property::kInteger)
			return false;
		inputColor = inputColorProp.getColor ();
		ignoreAlpha = ignoreAlphaProp.getInteger () > 0;
		return SimpleFilter<SimpleFilterProcessFunction>::run (replace);
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
	: SimpleFilter<SimpleFilterProcessFunction> ("A Grayscale Filter", processGrayscale)
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
	: SimpleFilter<SimpleFilterProcessFunction> ("A Replace Color Filter", processReplace)
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

	bool run (bool replace) override
	{
		const auto& inputColorProp = getProperty (Property::kInputColor);
		const auto& outputColorProp = getProperty (Property::kOutputColor);
		if (inputColorProp.getType () != BitmapFilter::Property::kColor ||
		    outputColorProp.getType () != BitmapFilter::Property::kColor)
			return false;
		inputColor = inputColorProp.getColor ();
		outputColor = outputColorProp.getColor ();
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

} // Standard

///@endcond

}} // namespaces
