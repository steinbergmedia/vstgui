//
//  ianimationtarget.h
//  vstgui
//
//  Created by Arne Scheffler on 06/08/14.
//
//

#ifndef __ianimationtarget__
#define __ianimationtarget__

#include "../vstguibase.h"

namespace VSTGUI {
namespace Animation {

//-----------------------------------------------------------------------------
/// @brief Animation target interface
///	@ingroup new_in_4_0
//-----------------------------------------------------------------------------
class IAnimationTarget
{
public:
	virtual ~IAnimationTarget () {}

	virtual void animationStart (CView* view, IdStringPtr name) = 0;						///< animation starts
	virtual void animationTick (CView* view, IdStringPtr name, float pos) = 0;				///< pos is a normalized value between zero and one
	virtual void animationFinished (CView* view, IdStringPtr name, bool wasCanceled) = 0;	///< animation ended
};

}} // namespaces

#endif // __ianimationtarget__
