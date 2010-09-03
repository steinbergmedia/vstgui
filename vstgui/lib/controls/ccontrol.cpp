//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins : 
//
// Version 4.0
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2010, Steinberg Media Technologies, All Rights Reserved
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
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A  PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#include "ccontrol.h"
#include "../cframe.h"

namespace VSTGUI {

//------------------------------------------------------------------------
/*! @defgroup views Views
 *	@ingroup viewsandcontrols
 */
//------------------------------------------------------------------------
/*! @defgroup controls Controls
 *	@ingroup views
 *	@brief Controls are views the user can interact with
 */
//------------------------------------------------------------------------
/*! @defgroup containerviews Container Views
 *	@ingroup views
 */
//------------------------------------------------------------------------

//------------------------------------------------------------------------
// CControl
//------------------------------------------------------------------------
/*! @class CControl
This object manages the tag identification and the value of a control object.
*/
CControl::CControl (const CRect& size, CControlListener* listener, int32_t tag, CBitmap *pBackground)
: CView (size)
, listener (listener)
, tag (tag)
, oldValue (1)
, defaultValue (0.5f)
, value (0)
, vmin (0)
, vmax (1.f)
, wheelInc (0.1f)
{
	setTransparency (false);
	setMouseEnabled (true);
	backOffset (0 ,0);

	setBackground (pBackground);
}

//------------------------------------------------------------------------
CControl::CControl (const CControl& c)
: CView (c)
, listener (c.listener)
, tag (c.tag)
, oldValue (c.oldValue)
, defaultValue (c.defaultValue)
, value (c.value)
, vmin (c.vmin)
, vmax (c.vmax)
, wheelInc (c.wheelInc)
{
}

//------------------------------------------------------------------------
CControl::~CControl ()
{
}

//------------------------------------------------------------------------
void CControl::setTag (int32_t val)
{
	if (listener)
		listener->controlTagWillChange (this);
	if (listeners.size () > 0)
	{
		std::list<CControlListener*>::const_iterator it = listeners.begin ();
		while (it != listeners.end ())
		{
			(*it)->controlTagWillChange (this);
			it++;
		}
	}
	tag = val;
	if (listener)
		listener->controlTagDidChange (this);
	if (listeners.size () > 0)
	{
		std::list<CControlListener*>::const_iterator it = listeners.begin ();
		while (it != listeners.end ())
		{
			(*it)->controlTagDidChange (this);
			it++;
		}
	}
}

//------------------------------------------------------------------------
void CControl::doIdleStuff ()
{
	if (pParentFrame)
		pParentFrame->doIdleStuff ();
}

//------------------------------------------------------------------------
void CControl::beginEdit ()
{
	// begin of edit parameter
	if (listener)
		listener->controlBeginEdit (this);
	if (listeners.size () > 0)
	{
		std::list<CControlListener*>::const_iterator it = listeners.begin ();
		while (it != listeners.end ())
		{
			(*it)->controlBeginEdit (this);
			it++;
		}
	}
	if (getFrame ())
		getFrame ()->beginEdit (tag);
}

//------------------------------------------------------------------------
void CControl::endEdit ()
{
	// end of edit parameter
	if (getFrame ())
		getFrame ()->endEdit (tag);
	if (listener)
		listener->controlEndEdit (this);
	if (listeners.size () > 0)
	{
		std::list<CControlListener*>::const_iterator it = listeners.begin ();
		while (it != listeners.end ())
		{
			(*it)->controlEndEdit (this);
			it++;
		}
	}
}

//------------------------------------------------------------------------
void CControl::setValue (float val, bool updateSubListeners)
{
	if (val < getMin ())
		val = getMin ();
	else if (val > getMax ())
		val = getMax ();
	if (val != value)
	{
		value = val;
		if (updateSubListeners)
		{
			std::list<CControlListener*>::const_iterator it = listeners.begin ();
			while (it != listeners.end ())
			{
				(*it)->valueChanged (this);
				it++;
			}
		}
	}
}

//------------------------------------------------------------------------
void CControl::setValueNormalized (float val, bool updateSubListeners)
{
	if (val > 1.f)
		val = 1.f;
	else if (val < 0.f)
		val = 0.f;
	setValue ((getMax () - getMin ()) * val + getMin (), updateSubListeners);
}

//------------------------------------------------------------------------
float CControl::getValueNormalized () const
{
	return (value - getMin ()) / (getMax () - getMin ());
}

//------------------------------------------------------------------------
void CControl::valueChanged ()
{
	if (listener)
		listener->valueChanged (this);
	if (listeners.size () > 0)
	{
		std::list<CControlListener*>::const_iterator it = listeners.begin ();
		while (it != listeners.end ())
		{
			(*it)->valueChanged (this);
			it++;
		}
	}
}

//------------------------------------------------------------------------
bool CControl::isDirty () const
{
	if (oldValue != value || CView::isDirty ())
		return true;
	return false;
}

//------------------------------------------------------------------------
void CControl::setDirty (const bool val)
{
	CView::setDirty (val);
	if (val)
	{
		if (value != -1.f)
			oldValue = -1.f;
		else
			oldValue = 0.f;
	}
	else
		oldValue = value;
}

//------------------------------------------------------------------------
void CControl::setBackOffset (const CPoint &offset)
{
	backOffset = offset;
}

//-----------------------------------------------------------------------------
void CControl::copyBackOffset ()
{
	backOffset (size.left, size.top);
}

//------------------------------------------------------------------------
void CControl::bounceValue ()
{
	if (value > getMax ())
		value = getMax ();
	else if (value < getMin ())
		value = getMin ();
}

//-----------------------------------------------------------------------------
bool CControl::checkDefaultValue (CButtonState button)
{
	if (getFrame () && getFrame ()->getEditor () && getFrame ()->getEditor ()->isControlDefaultButton (button))
	{
		// begin of edit parameter
		beginEdit ();
	
		value = getDefaultValue ();
		if (isDirty () && listener)
			listener->valueChanged (this);

		// end of edit parameter
		endEdit ();
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
bool CControl::drawFocusOnTop ()
{
	return false;
}

//------------------------------------------------------------------------
bool CControl::getFocusPath (CGraphicsPath& outPath)
{
	if (wantsFocus ())
	{
		CCoord focusWidth = getFrame ()->getFocusWidth ();
		CRect r (getVisibleSize ());
		if (!r.isEmpty ())
		{
			outPath.addRect (r);
			r.inset (-focusWidth, -focusWidth);
			outPath.addRect (r);
		}
	}
	return true;
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
void IMultiBitmapControl::autoComputeHeightOfOneImage ()
{
	CView* view = dynamic_cast<CView*>(this);
	if (view)
	{
		CRect viewSize = view->getViewSize (viewSize);
		heightOfOneImage = viewSize.height ();
	}
}

} // namespace
