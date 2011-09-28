#ifndef __cshadowviewcontainer__
#define __cshadowviewcontainer__

#include "cviewcontainer.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
class CShadowViewContainer : public CViewContainer
{
public:
	CShadowViewContainer (const CRect& size);
	CShadowViewContainer (const CShadowViewContainer& copy);

	virtual void setShadowOffset (const CPoint& offset);
	const CPoint& getShadowOffset () const { return shadowOffset; }
	
	virtual void setShadowIntensity (float intensity);
	float getShadowIntensity () const { return shadowIntensity; }

	virtual void setShadowBlurSize (uint32_t size);
	uint32_t getShadowBlurSize () const { return shadowBlurSize; }

	void invalidateShadow ();

	// override
	void drawRect (CDrawContext* pContext, const CRect& updateRect);
	void drawBackgroundRect (CDrawContext* pContext, const CRect& _updateRect);
	void setViewSize (const CRect& rect, bool invalid = true);
	bool addView (CView* pView);
	bool addView (CView* pView, const CRect& mouseableArea, bool mouseEnabled = true);
	bool addView (CView* pView, CView* pBefore);
	bool removeView (CView* pView, bool withForget = true);
	bool changeViewZOrder (CView* view, int32_t newIndex);
	CMessageResult notify (CBaseObject* sender, IdStringPtr message);

	CLASS_METHODS(CShadowViewContainer, CViewContainer)
protected:
	bool shadowInvalid;
	CPoint shadowOffset;
	float shadowIntensity;
	uint32_t shadowBlurSize;
};

} // namespace

#endif // __cshadowviewcontainer__
