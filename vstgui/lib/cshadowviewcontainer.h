// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __cshadowviewcontainer__
#define __cshadowviewcontainer__

#include "cviewcontainer.h"
#include "iviewlistener.h"
#include "iscalefactorchangedlistener.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
// CShadowViewContainer Declaration
//! @brief a view container which draws a shadow for it's subviews
/// @ingroup containerviews
/// @ingroup new_in_4_1
//-----------------------------------------------------------------------------
class CShadowViewContainer : public CViewContainer, public IScaleFactorChangedListener, public IViewContainerListenerAdapter
{
public:
	explicit CShadowViewContainer (const CRect& size);
	CShadowViewContainer (const CShadowViewContainer& copy);
	~CShadowViewContainer () noexcept override;

	//-----------------------------------------------------------------------------
	/// @name CShadowViewContainer Methods
	//-----------------------------------------------------------------------------
	//@{
	virtual void setShadowOffset (const CPoint& offset);
	const CPoint& getShadowOffset () const { return shadowOffset; }
	
	virtual void setShadowIntensity (float intensity);
	float getShadowIntensity () const { return shadowIntensity; }

	virtual void setShadowBlurSize (double size);
	double getShadowBlurSize () const { return shadowBlurSize; }

	void invalidateShadow ();
	//@}

	// override
	bool removed (CView* parent) override;
	bool attached (CView* parent) override;
	void drawRect (CDrawContext* pContext, const CRect& updateRect) override;
	void drawBackgroundRect (CDrawContext* pContext, const CRect& _updateRect) override;
	void setViewSize (const CRect& rect, bool invalid = true) override;
	CMessageResult notify (CBaseObject* sender, IdStringPtr message) override;

	void onScaleFactorChanged (CFrame* frame, double newScaleFactor) override;

	CLASS_METHODS(CShadowViewContainer, CViewContainer)
protected:
	void viewContainerViewAdded (CViewContainer* container, CView* view) override;
	void viewContainerViewRemoved (CViewContainer* container, CView* view) override;
	void viewContainerViewZOrderChanged (CViewContainer* container, CView* view) override;

	void beforeDelete () override;

	bool dontDrawBackground;
	CPoint shadowOffset;
	float shadowIntensity;
	double shadowBlurSize;
	double scaleFactorUsed;
};

} // namespace

#endif // __cshadowviewcontainer__
