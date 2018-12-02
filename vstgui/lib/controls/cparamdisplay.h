// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "ccontrol.h"
#include "../cfont.h"
#include "../ccolor.h"
#include "../cdrawdefs.h"
#include <functional>

namespace VSTGUI {

//-----------------------------------------------------------------------------
using CParamDisplayValueToStringProc = bool (*) (float value, char utf8String[256], void* userData);

//-----------------------------------------------------------------------------
// CParamDisplay Declaration
//! @brief a parameter display
/// @ingroup views
//-----------------------------------------------------------------------------
class CParamDisplay : public CControl
{
protected:
	enum StyleEnum
	{
		StyleShadowText = 0,
		Style3DIn,
		Style3DOut,
		StyleNoText,
		StyleNoDraw,
		StyleRoundRect,
		StyleNoFrame,
		StyleAntialias,
		LastStyle
	};

public:
	CParamDisplay (const CRect& size, CBitmap* background = nullptr, int32_t style = 0);
	CParamDisplay (const CParamDisplay& paramDisplay);
	
	//-----------------------------------------------------------------------------
	/// @name CParamDisplay Methods
	//-----------------------------------------------------------------------------
	//@{
	virtual void setFont (CFontRef fontID);
	const CFontRef getFont () const { return fontID; }

	virtual void setFontColor (CColor color);
	CColor getFontColor () const { return fontColor; }

	virtual void setBackColor (CColor color);
	CColor getBackColor () const { return backColor; }

	virtual void setFrameColor (CColor color);
	CColor getFrameColor () const { return frameColor; }

	virtual void setShadowColor (CColor color);
	CColor getShadowColor () const { return shadowColor; }

	virtual void setShadowTextOffset (const CPoint& offset);
	CPoint getShadowTextOffset () const { return shadowTextOffset; }

	virtual void setAntialias (bool state) { setBit (style, kAntialias, state); }
	bool getAntialias () const { return hasBit (style, kAntialias); }

	virtual void setHoriAlign (CHoriTxtAlign hAlign);
	CHoriTxtAlign getHoriAlign () const { return horiTxtAlign; }

	virtual void setTextInset (const CPoint& p);
	CPoint getTextInset () const { return textInset; }

	virtual void setTextRotation (double angle);
	double getTextRotation () const { return textRotation; }

	virtual void setRoundRectRadius (const CCoord& radius);
	CCoord getRoundRectRadius () const { return roundRectRadius; }

	virtual void setFrameWidth (const CCoord& width);
	CCoord getFrameWidth () const { return frameWidth; }

	using ValueToStringUserData = CParamDisplay;
	using ValueToStringFunction = std::function<bool (float value, char utf8String[256], CParamDisplay* display)>;
	
	void setValueToStringFunction (const ValueToStringFunction& valueToStringFunc);
	void setValueToStringFunction (ValueToStringFunction&& valueToStringFunc);

	using ValueToStringFunction2 = std::function<bool (float value, std::string& result, CParamDisplay* display)>;

	void setValueToStringFunction2 (const ValueToStringFunction2& valueToStringFunc);
	void setValueToStringFunction2 (ValueToStringFunction2&& valueToStringFunc);
	
	enum Style
	{
		kShadowText		= 1 << StyleShadowText,
		k3DIn			= 1 << Style3DIn,
		k3DOut			= 1 << Style3DOut,
		kNoTextStyle	= 1 << StyleNoText,
		kNoDrawStyle	= 1 << StyleNoDraw,
		kRoundRectStyle = 1 << StyleRoundRect,
		kNoFrame		= 1 << StyleNoFrame,
	};
	virtual void setStyle (int32_t val);
	int32_t getStyle () const;

	virtual void setPrecision (uint8_t precision);
	uint8_t getPrecision () const { return valuePrecision; }

	virtual void setBackOffset (const CPoint& offset);
	const CPoint& getBackOffset () const { return backOffset; }
	void copyBackOffset ();

	//@}

	void draw (CDrawContext* pContext) override;
	bool getFocusPath (CGraphicsPath& outPath) override;
	bool removed (CView* parent) override;

	CLASS_METHODS(CParamDisplay, CControl)
protected:
	~CParamDisplay () noexcept override;
	virtual void drawBack (CDrawContext* pContext, CBitmap* newBack = nullptr);

	virtual void drawPlatformText (CDrawContext* pContext, IPlatformString* string);
	virtual void drawPlatformText (CDrawContext* pContext, IPlatformString* string, const CRect& size);

	virtual void drawStyleChanged ();

	ValueToStringFunction2 valueToStringFunction;

	enum StylePrivate {
		kAntialias		= 1 << StyleAntialias,
	};

	CHoriTxtAlign horiTxtAlign;
	int32_t		style;
	uint8_t		valuePrecision;

	CFontRef	fontID;
	CColor		fontColor;
	CColor		backColor;
	CColor		frameColor;
	CColor		shadowColor;
	CPoint		textInset;
	CPoint		shadowTextOffset {1., 1.};
	CPoint		backOffset;
	CCoord		roundRectRadius;
	CCoord		frameWidth;
	double		textRotation;
};

} // VSTGUI
