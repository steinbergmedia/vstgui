#ifndef __cbitmapfilter__
#define __cbitmapfilter__

#include "cbitmap.h"
#include <vector>
#include <string>
#include <map>

namespace VSTGUI {
class CBitmap;
class CBitmapPixelAccess;
struct CColor;
struct CRect;
struct CPoint;
struct CGraphicsTransform;

namespace BitmapFilter {

//----------------------------------------------------------------------------------------------------
/// @brief Filter Property
/// @ingroup new_in_4_1
//----------------------------------------------------------------------------------------------------
class Property : public CBaseObject
{
public:
	enum Type {
		kNotFound = 0,
		kInteger,
		kFloat,
		kObject,
		kRect,
		kPoint,
		kColor,
		kTransformMatrix
	};
	
	Property (Type type);
	Property (int32_t intValue);
	Property (double floatValue);
	Property (CBaseObject* objectValue);
	Property (const CRect& rectValue);
	Property (const CPoint& pointValue);
	Property (const CColor& colorValue);
	Property (const CGraphicsTransform& transformValue);
	Property (const Property& p);
	~Property ();

	Type getType () const { return type; }

	int32_t getInteger () const;
	double getFloat () const;
	CBaseObject* getObject () const;
	const CRect& getRect () const;
	const CPoint& getPoint () const;
	const CColor& getColor () const;
	const CGraphicsTransform& getTransform () const;

	Property& operator=(const Property& p);

	CLASS_METHODS_NOCOPY(Property, CBaseObject)
//----------------------------------------------------------------------------------------------------
private:
	Type type;
	void* value;
};

//----------------------------------------------------------------------------------------------------
/// @brief Filter Interface
/// @ingroup new_in_4_1
//----------------------------------------------------------------------------------------------------
class IFilter : public CBaseObject
{
public:
	virtual bool run (bool replaceInputBitmap = false) = 0;
	
	virtual UTF8StringPtr getDescription () const = 0;
	virtual bool setProperty (IdStringPtr name, const Property& property) = 0;
	virtual const Property& getProperty (IdStringPtr name) const = 0;

	virtual uint32_t getNumProperties () const = 0;
	virtual IdStringPtr getPropertyName (uint32_t index) const = 0;
	virtual Property::Type getPropertyType (uint32_t index) const = 0;
	virtual Property::Type getPropertyType (IdStringPtr name) const = 0;

	typedef IFilter* (*CreateFunction) (IdStringPtr name);

	CLASS_METHODS_NOCOPY(IFilter, CBaseObject)
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
	std::vector<std::pair<std::string, IFilter::CreateFunction> > filters;
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

	/** @brief Standard Bitmap Property Names */
	namespace Property {
	
		static const IdStringPtr kInputBitmap = "InputBitmap"; ///< [Property::kObject]
		static const IdStringPtr kOutputBitmap = "OutputBitmap"; ///< [Property::kObject]
		static const IdStringPtr kRadius = "Radius"; //< [Property::kInteger]
		static const IdStringPtr kInputColor = "InputColor"; ///< [Property::kColor]
		static const IdStringPtr kOutputColor = "OutputColor"; ///< [Property::kColor]
		static const IdStringPtr kOutputRect = "OutputRect"; ///< [Property::kRect]
		static const IdStringPtr kIgnoreAlphaColorValue = "IgnoreAlphaColorValue"; ///< [Property::kInteger]

	} // namespace Property

} // namespace Standard

//----------------------------------------------------------------------------------------------------
/// @brief A Base Class for Implementing Bitmap Filters
/// @ingroup new_in_4_1
//----------------------------------------------------------------------------------------------------
class FilterBase : public IFilter, private std::map<std::string, Property>
{
protected:
	FilterBase (UTF8StringPtr description);

	bool registerProperty (IdStringPtr name, const Property& defaultProperty);
	CBitmap* getInputBitmap () const;

	virtual UTF8StringPtr getDescription () const;
	virtual bool setProperty (IdStringPtr name, const Property& property);
	virtual const Property& getProperty (IdStringPtr name) const;
	
	virtual uint32_t getNumProperties () const;
	virtual IdStringPtr getPropertyName (uint32_t index) const;
	virtual Property::Type getPropertyType (uint32_t index) const;
	virtual Property::Type getPropertyType (IdStringPtr name) const;

private:
	std::string description;
};

} // namespace BitmapFilter

} // namespace

#endif // __cbitmapfilter__
