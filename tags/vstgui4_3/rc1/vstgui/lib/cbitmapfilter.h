//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 4.3
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2015, Steinberg Media Technologies, All Rights Reserved
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

#ifndef __cbitmapfilter__
#define __cbitmapfilter__

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
class Property : public CBaseObject
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
	Property (CBaseObject* objectValue);
	Property (const CRect& rectValue);
	Property (const CPoint& pointValue);
	Property (const CColor& colorValue);
	Property (const CGraphicsTransform& transformValue);
	Property (const Property& p);
	~Property ();

#if VSTGUI_RVALUE_REF_SUPPORT
	Property (Property&& p) noexcept;
	Property& operator=(Property&& p) noexcept;
#endif
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
	template<typename T> void assign (T value);
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
	typedef std::map<std::string, IFilter::CreateFunction > FilterMap;
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
	namespace Property {
	
		static const IdStringPtr kInputBitmap = "InputBitmap"; ///< [Property::kObject - CBitmap]
		static const IdStringPtr kOutputBitmap = "OutputBitmap"; ///< [Property::kObject - CBitmap]
		static const IdStringPtr kRadius = "Radius"; ///< [Property::kInteger]
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

	virtual UTF8StringPtr getDescription () const VSTGUI_OVERRIDE_VMETHOD;
	virtual bool setProperty (IdStringPtr name, const Property& property) VSTGUI_OVERRIDE_VMETHOD;
	virtual const Property& getProperty (IdStringPtr name) const VSTGUI_OVERRIDE_VMETHOD;
	
	virtual uint32_t getNumProperties () const VSTGUI_OVERRIDE_VMETHOD;
	virtual IdStringPtr getPropertyName (uint32_t index) const VSTGUI_OVERRIDE_VMETHOD;
	virtual Property::Type getPropertyType (uint32_t index) const VSTGUI_OVERRIDE_VMETHOD;
	virtual Property::Type getPropertyType (IdStringPtr name) const VSTGUI_OVERRIDE_VMETHOD;

private:
	std::string description;
};

} // namespace BitmapFilter

} // namespace

#endif // __cbitmapfilter__
