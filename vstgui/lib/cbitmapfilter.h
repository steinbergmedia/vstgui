#ifndef __cbitmapfilter__
#define __cbitmapfilter__

#include "cbitmap.h"

namespace VSTGUI {
class CBitmap;
class CBitmapPixelAccess;
struct CColor;

namespace BitmapFilter {

//----------------------------------------------------------------------------------------------------
/// @brief Box Blur Bitmap Filter
/// @ingroup new_in_4_1
//----------------------------------------------------------------------------------------------------
class BoxBlur
{
public:
	/** adds a box blur to the bitmap */
	static bool process (CBitmap* bitmap, uint32_t boxSize);
	static CBitmapPixelAccess& process (CBitmapPixelAccess& accessor, uint32_t boxSize);
protected:
	/// @cond ignore
	static inline void calculate (CColor& color, CColor* colors, uint32_t numColors);
	/// @endcond ignore
};

//----------------------------------------------------------------------------------------------------
/// @brief Grayscale Bitmap Filter
/// @ingroup new_in_4_1
//----------------------------------------------------------------------------------------------------
class Grayscale
{
public:
	/** replaces colors to grayscale */
	static bool process (CBitmap* bitmap);
	static CBitmapPixelAccess& process (CBitmapPixelAccess& accessor);
};

//----------------------------------------------------------------------------------------------------
/// @brief Copy Bitmap Filter
/// @ingroup new_in_4_1
//----------------------------------------------------------------------------------------------------
class Copy
{
public:
	/** creates a copy of the bitmap. The result is owned by the caller. */
	static CBitmap* process (CBitmap* bitmap);
	static CBitmapPixelAccess& process (CBitmapPixelAccess& originalBitmapAccessor, CBitmapPixelAccess& copyBitmapAccessor);
};

//----------------------------------------------------------------------------------------------------
/// @brief Extract Bitmap Filter
/// @ingroup new_in_4_1
//----------------------------------------------------------------------------------------------------
class Extract
{
public:
	/** extracts a part of the bitmap. The result is owned by the caller. */
	static CBitmap* process (CBitmap* bitmap, const CRect& size);
	static CBitmapPixelAccess& process (CBitmapPixelAccess& originalBitmapAccessor, CBitmapPixelAccess& copyBitmapAccessor, const CPoint& leftTop);
};

//----------------------------------------------------------------------------------------------------
/// @brief Expand Bitmap Filter
/// @ingroup new_in_4_1
//----------------------------------------------------------------------------------------------------
class Expand
{
public:
	/** expands the bitmap by adding transparent pixels. The result is owned by the caller. */
	static CBitmap* process (CBitmap* bitmap, CCoord left = 0, CCoord top = 0, CCoord right = 0, CCoord bottom = 0);
	static CBitmapPixelAccess& process (CBitmapPixelAccess& originalBitmapAccessor, CBitmapPixelAccess& copyBitmapAccessor, CCoord left = 0, CCoord top = 0, CCoord right = 0, CCoord bottom = 0);
};

//----------------------------------------------------------------------------------------------------
/// @brief Scale Bitmap Filter
/// @ingroup new_in_4_1
//----------------------------------------------------------------------------------------------------
class Scale
{
public:
	/** creates a bilinear scaled copy of the bitmap. The result is owned by the caller. */
	static CBitmap* processBilinear (CBitmap* bitmap, const CPoint& newSize);
	static CBitmapPixelAccess& processBilinear (CBitmapPixelAccess& originalBitmapAccessor, CBitmapPixelAccess& copyBitmapAccessor);
};

} // namespace BitmapFilter

} // namespace

#endif // __cbitmapfilter__
