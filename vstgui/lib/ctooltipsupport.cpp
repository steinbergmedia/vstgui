// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "ctooltipsupport.h"
#include "cframe.h"
#include "cvstguitimer.h"
#include "platform/iplatformframe.h"

#if DEBUG
#define DEBUGLOG 0
#endif

namespace VSTGUI {

//------------------------------------------------------------------------
/*! @class VSTGUI::CTooltipSupport
A generic way to add tooltip support to VSTGUI.
@section ctooltipsupport_example Example
Adding a tooltip to a view
@code
UTF8StringPtr tooltipText = "This is a tooltip";
view->setAttribute (kCViewTooltipAttribute, strlen (tooltipText)+1, tooltipText);
@endcode
Adding CTooltipSupport is done via VSTGUI::CFrame::enableTooltips (true) */
//------------------------------------------------------------------------
/**
 * @param frame CFrame object
 * @param delay tooltip delay time in milliseconds
 */
CTooltipSupport::CTooltipSupport (CFrame* frame, uint32_t delay)
: timer (nullptr)
, frame (frame)
, currentView (nullptr)
, delay (delay)
, state (kHidden)
{
	timer = makeOwned<CVSTGUITimer> (this, delay);
}

//------------------------------------------------------------------------
CTooltipSupport::~CTooltipSupport () noexcept
{
	IPlatformFrame* platformFrame = frame->getPlatformFrame ();
	if (platformFrame)
		platformFrame->hideTooltip ();
}

//------------------------------------------------------------------------
static UTF8StringBuffer getTooltipFromView (CView* view)
{
	UTF8StringBuffer tooltip = nullptr;
	uint32_t tooltipSize = 0;
	if (view->getAttributeSize (kCViewTooltipAttribute, tooltipSize))
	{
		tooltip = (UTF8StringBuffer)std::malloc (tooltipSize + 1);
		memset (tooltip, 0, tooltipSize + 1);
		if (!view->getAttribute (kCViewTooltipAttribute, tooltipSize, tooltip, tooltipSize))
		{
			std::free (tooltip);
			tooltip = nullptr;
		}
	}
	return tooltip;
}

//------------------------------------------------------------------------
static bool viewHasTooltip (CView* view)
{
	uint32_t tooltipSize = 0;
	if (view->getAttributeSize (kCViewTooltipAttribute, tooltipSize))
	{
		if (tooltipSize > 0)
			return true;
	}
	return false;
}

//------------------------------------------------------------------------
void CTooltipSupport::onMouseEntered (CView* view)
{
	if (viewHasTooltip (view))
	{
		currentView = view;
		if (state == kHiding)
		{
			#if DEBUGLOG
			DebugPrint ("CTooltipSupport::onMouseEntered (%s) - show tooltip\n", view->getClassName ());
			#endif
			state = kShowing;
			timer->setFireTime (50);
			timer->start ();
		}
		else if (state == kHidden)
		{
			#if DEBUGLOG
			DebugPrint ("CTooltipSupport::onMouseEntered (%s) - start timer\n", view->getClassName ());
			#endif
			state = kShowing;
			timer->setFireTime (delay);
			timer->start ();
		}
		else
		{
			#if DEBUGLOG
			DebugPrint ("CTooltipSupport::onMouseEntered (%s) - unexpected internal state\n", view->getClassName ());
			#endif
		}
	}
}

//------------------------------------------------------------------------
void CTooltipSupport::onMouseExited (CView* view)
{
	if (currentView == view)
	{
		if (state == kHidden || state == kShowing)
		{
			hideTooltip ();
			timer->setFireTime (delay);
		}
		else
		{
			state = kHiding;
			timer->setFireTime (200);
			timer->start ();
		}
		currentView = nullptr;
		#if DEBUGLOG
		DebugPrint ("CTooltipSupport::onMouseExited (%s)\n", view->getClassName ());
		#endif
	}
}

//------------------------------------------------------------------------
void CTooltipSupport::onMouseMoved (const CPoint& where)
{
	if (currentView && state != kForceVisible)
	{
		CRect r (lastMouseMove.x-2, lastMouseMove.y-2, lastMouseMove.x+2, lastMouseMove.y+2);
		if (!r.pointInside (where))
		{
			if (state == kHidden)
			{
				if (timer->stop ())
				{
					#if DEBUGLOG
					DebugPrint ("CTooltipSupport::onMouseMoved (%s) - Timer restarted\n", currentView->getClassName ());
					#endif
					timer->start ();
				}
			}
			else if (state == kVisible)
			{
				#if DEBUGLOG
				DebugPrint ("CTooltipSupport::onMouseMoved (%s) - will hide tooltip\n", currentView->getClassName ());
				#endif
				state = kHiding;
				timer->setFireTime (200);
				timer->start ();
			}
			else
			{
				#if DEBUGLOG
				DebugPrint ("CTooltipSupport::onMouseMoved (%s) - state: %d\n", currentView->getClassName (), state);
				#endif
			}
		}
		else
		{
			#if DEBUGLOG
			DebugPrint ("CTooltipSupport::onMouseMoved (%s) - small move\n", currentView->getClassName ());
			#endif
		}
	}
	lastMouseMove = where;
}

//------------------------------------------------------------------------
void CTooltipSupport::onMouseDown (const CPoint& where)
{
	if (state != kHidden)
	{
		hideTooltip ();
		timer->setFireTime (delay);
	}
}

//------------------------------------------------------------------------
void CTooltipSupport::hideTooltip ()
{
	state = kHidden;
	timer->stop ();
	IPlatformFrame* platformFrame = frame->getPlatformFrame ();
	if (platformFrame)
		platformFrame->hideTooltip ();

	#if DEBUGLOG
	DebugPrint ("CTooltipSupport::hideTooltip\n");
	#endif
}

//------------------------------------------------------------------------
bool CTooltipSupport::showTooltip ()
{
	if (currentView)
	{
		if (currentView->isAttached () == false)
		{
			currentView = nullptr;
			return false;
		}
		CRect r = currentView->translateToGlobal (currentView->getVisibleViewSize ());

		UTF8StringBuffer tooltip = getTooltipFromView (currentView);
		
		if (tooltip)
		{
			state = kForceVisible;
			
			IPlatformFrame* platformFrame = frame->getPlatformFrame ();
			if (platformFrame)
				platformFrame->showTooltip (r, tooltip);

			std::free (tooltip);

			#if DEBUGLOG
			DebugPrint ("CTooltipSupport::showTooltip (%s)\n", currentView->getClassName ());
			#endif
			return true;
		}
	}
	return false;
}

//------------------------------------------------------------------------
CMessageResult CTooltipSupport::notify (CBaseObject* sender, IdStringPtr msg)
{
	if (msg == CVSTGUITimer::kMsgTimer)
	{
		if (state == kHiding)
		{
			hideTooltip ();
			timer->setFireTime (delay);
		}
		else if (state == kShowing)
		{
			if (showTooltip ())
			{
				timer->setFireTime (100);
			}
			else
			{
				state = kHidden;
				timer->stop ();
			}
		}
		else if (state == kForceVisible)
		{
			state = kVisible;
			timer->stop ();
			timer->setFireTime (delay);
		}
		return kMessageNotified;
	}
	return kMessageUnknown;
}

} // namespace
