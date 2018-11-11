// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "ccontrol.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
// CSpecialDigit Declaration
//! @brief special display with custom numbers (0...9)
/// @ingroup views
//-----------------------------------------------------------------------------
class CSpecialDigit : public CControl
{
public:
	CSpecialDigit (const CRect& size, IControlListener* listener, int32_t tag, int32_t dwPos, int32_t iNumbers, int32_t* xpos, int32_t* ypos, int32_t width, int32_t height, CBitmap* background);
	CSpecialDigit (const CSpecialDigit& digit);
	
	void  draw (CDrawContext*) override;

	CLASS_METHODS(CSpecialDigit, CControl)
protected:
	~CSpecialDigit () noexcept override = default;
	int32_t     iNumbers;
	int32_t     xpos[7];
	int32_t     ypos[7];
	int32_t     width;
	int32_t     height;
};

} // VSTGUI
