// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "vstguifwd.h"
#include <vector>
#include <string>
#include <map>

namespace VSTGUI {

namespace BitmapFilter {

//----------------------------------------------------------------------------------------------------
/// @brief Filter Property
/// @ingroup new_in_4_1
//----------------------------------------------------------------------------------------------------
class Property
{
public:
	enum Type {
		kUnknown = 0,
		kInteger,
		kFloat,
		kObject,
		kRect,
		kPoint,
		kColor,
		kTransformMatrix
	};
	
	Property (Type type = kUnknown);
	Property (int32_t intValue);
	Property (double floatValue);
	Property (IReference* objectValue);
	Property (const CRect& rectValue);
	Property (const CPoint& pointValue);
	Property (const CColor& colorValue);
	Property (const CGraphicsTransform& transformValue);
	Property (const Property& p);
	Property (Property&& p) noexcept;
	~Property () noexcept;

	Type getType () const { return type; }

	int32_t getInteger () const;
	double getFloat () const;
	IReference* getObject () const;
	const CRect& getRect () const;
	const CPoint& getPoint () const;
	const CColor& getColor () const;
	const CGraphicsTransform& getTransform () const;

	Property& operator=(const Property& p);
	Property& operator=(Property&& p) noexcept;

//----------------------------------------------------------------------------------------------------
private:
	template<typename T> void assign (T value);
	Type type;
	void* value;
};

//----------------------------------------------------------------------------------------------------
/// @brief Filter Interface
/// @ingroup new_in_4_1
//----------------------------------------------------------------------------------------------------
class IFilter : public NonAtomicReferenceCounted
{
public:
	virtual bool run (bool replaceInputBitmap = false) = 0;
	
	virtual UTF8StringPtr getDescription () const = 0;
	virtual bool setProperty (IdStringPtr name, const Property& property) = 0;
	virtual bool setProperty (IdStringPtr name, Property&& property) = 0;
	virtual const Property& getProperty (IdStringPtr name) const = 0;

	virtual uint32_t getNumProperties () const = 0;
	virtual IdStringPtr getPropertyName (uint32_t index) const = 0;
	virtual Property::Type getPropertyType (uint32_t index) const = 0;
	virtual Property::Type getPropertyType (IdStringPtr name) const = 0;

	using CreateFunction = IFilter* (*) (IdStringPtr name);
};

//----------------------------------------------------------------------------------------------------
/// @brief Bitmap Filter Factory.
/// @ingroup new_in_4_1
/// @details See @ref VSTGUI::BitmapFilter::Standard for a description of included Filters
//----------------------------------------------------------------------------------------------------
class Factory
{
public:
	static Factory& getInstance ();
	
	uint32_t getNumFilters () const;
	IdStringPtr getFilterName (uint32_t index) const;
	
	IFilter* createFilter (IdStringPtr name) const;
	
	bool registerFilter (IdStringPtr name, IFilter::CreateFunction createFunction);
	bool unregisterFilter (IdStringPtr name, IFilter::CreateFunction createFunction);
protected:
	using FilterMap = std::map<std::string, IFilter::CreateFunction>;
	FilterMap filters;
};

/** @brief Standard Bitmap Filter Names */
namespace Standard {

	/** Box Blur Filter Name.

		Applies a box blur on the input bitmap.

		Properties:
			- Property::kInputBitmap
			- Property::kRadius
			- Property::kOutputBitmap
	*/
	static const IdStringPtr kBoxBlur = "Box Blur";

	/** Grayscale Filter Name.
	 
		Produces a grayscale version of the input bitmap.

		Properties:
			- Property::kInputBitmap
			- Property::kOutputBitmap
			- Property::kAlphaChannelOnly
	 */
	static const IdStringPtr kGrayscale = "Grayscale";

	/** Replace Color Filter Name.
	 
		Replaces the colors which match the input color to the output color.

		Properties:
			- Property::kInputBitmap
			- Property::kInputColor
			- Property::kOutputColor
			- Property::kOutputBitmap
	 */
	static const IdStringPtr kReplaceColor = "Replace Color";

	/** Set Color Filter Name.
	 
		Sets all colors of the input bitmap the color of the input color. If Property::kIgnoreAlphaColorValue is set, the
		alpha value of the input bitmap is not changed.
		 
		Properties:
			- Property::kInputBitmap
			- Property::kInputColor
			- Property::kIgnoreAlphaColorValue
			- Property::kOutputBitmap
	 */
	static const IdStringPtr kSetColor = "Set Color";

	/** Scale Bilinear Filter Name.
	 
		Creates a bilinear scaled bitmap of the input bitmap.
		Does not work inplace.

		Properties:
			- Property::kInputBitmap
			- Property::kOutputRect
			- Property::kOutputBitmap
	 */
	static const IdStringPtr kScaleBilinear = "Scale Biliniear";

	/** Scale Linear Filter Name.
	 
		Creates a linear scaled bitmap of the input bitmap.
		Does not work inplace.

		Properties:
			- Property::kInputBitmap
			- Property::kOutputRect
			- Property::kOutputBitmap
	 */
	static const IdStringPtr kScaleLinear = "Scale Linear";

	/** @brief Standard Bitmap Property Names */
	namespace Property
	{
		/** [Property::kObject - CBitmap] */
		static const IdStringPtr kInputBitmap = "InputBitmap";
		/** [Property::kObject - CBitmap] */
		static const IdStringPtr kOutputBitmap = "OutputBitmap";
		/** [Property::kInteger] */
		static const IdStringPtr kRadius = "Radius";
		/** [Property::kColor] */
		static const IdStringPtr kInputColor = "InputColor";
		/** [Property::kColor] */
		static const IdStringPtr kOutputColor = "OutputColor";
		/** [Property::kRect] */
		static const IdStringPtr kOutputRect = "OutputRect";
		/** [Property::kInteger] */
		static const IdStringPtr kIgnoreAlphaColorValue = "IgnoreAlphaColorValue";
		/** [Property::kInteger] */
		static const IdStringPtr kAlphaChannelOnly = "AlphaChannelOnly";
	} // Property

} // Standard

//----------------------------------------------------------------------------------------------------
/// @brief A Base Class for Implementing Bitmap Filters
/// @ingroup new_in_4_1
//----------------------------------------------------------------------------------------------------
class FilterBase : public IFilter
{
protected:
	FilterBase (UTF8StringPtr description);

	bool registerProperty (IdStringPtr name, const Property& defaultProperty);
	CBitmap* getInputBitmap () const;

	UTF8StringPtr getDescription () const override;
	bool setProperty (IdStringPtr name, const Property& property) override;
	bool setProperty (IdStringPtr name, Property&& property) override;
	const Property& getProperty (IdStringPtr name) const override;
	
	uint32_t getNumProperties () const override;
	IdStringPtr getPropertyName (uint32_t index) const override;
	Property::Type getPropertyType (uint32_t index) const override;
	Property::Type getPropertyType (IdStringPtr name) const override;

private:
	using PropertyMap = std::map<std::string, Property>;
	std::string description;
	PropertyMap properties;
};

} // BitmapFilter
} // VSTGUI
