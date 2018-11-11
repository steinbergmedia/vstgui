// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "vstguifwd.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
// IFocusDrawing Declaration
/// @brief Custom focus drawing interface
///	@ingroup new_in_4_0
///
/// @details If focus drawing is enabled custom views can implement this interface to set a custom shape to be drawn if the view is the focus view.
/// @sa CFrame
/// @sa CControl
//-----------------------------------------------------------------------------
class IFocusDrawing
{
public:
	virtual ~IFocusDrawing () noexcept = default;
	/** draw focus before view will be drawn or afterwards */
	virtual bool drawFocusOnTop () = 0;
	/** the graphics path will be drawn filled with the evenodd method and the color set in CFrame::setFocusColor() */
	virtual bool getFocusPath (CGraphicsPath& outPath) = 0;
};

} // VSTGUI
