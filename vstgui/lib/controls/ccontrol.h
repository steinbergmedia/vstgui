// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../cview.h"
#include "../ifocusdrawing.h"
#include "../idependency.h"
#include "../dispatchlist.h"
#include "icontrollistener.h"
#include <list>

namespace VSTGUI {
namespace Constants {

static constexpr auto pi = 3.14159265358979323846;
static constexpr auto double_pi = 6.28318530717958647692;
static constexpr auto half_pi = 1.57079632679489661923f;
static constexpr auto quarter_pi = 0.78539816339744830962;
static constexpr auto e = 2.7182818284590452354;
static constexpr auto ln2 = 0.69314718055994530942;
static constexpr auto sqrt2 = 1.41421356237309504880;

} // Constants

//-----------------------------------------------------------------------------
// CControl Declaration
//! @brief base class of all VSTGUI controls
//-----------------------------------------------------------------------------
class CControl : public CView, public IFocusDrawing
{
public:
	CControl (const CRect& size, IControlListener* listener = nullptr, int32_t tag = 0, CBitmap* pBackground = nullptr);
	CControl (const CControl& c);

	//-----------------------------------------------------------------------------
	/// @name Value Methods
	//-----------------------------------------------------------------------------
	//@{
	virtual void setValue (float val);
	virtual float getValue () const { return value; }

	virtual void setValueNormalized (float val);
	virtual float getValueNormalized () const;

	virtual void setMin (float val) { vmin = val; bounceValue (); }
	virtual float getMin () const { return vmin; }
	virtual void setMax (float val) { vmax = val; bounceValue (); }
	virtual float getMax () const { return vmax; }
	float getRange () const { return getMax () - getMin (); }

	virtual void setOldValue (float val) { oldValue = val; }
	virtual	float getOldValue (void) const { return oldValue; }
	virtual void setDefaultValue (float val) { defaultValue = val; }
	virtual	float getDefaultValue (void) const { return defaultValue; }

	virtual void bounceValue ();
	virtual bool checkDefaultValue (CButtonState button);
	
	/** notifies listener and dependent objects */
	virtual void valueChanged ();
	//@}

	//-----------------------------------------------------------------------------
	/// @name Editing Methods
	//-----------------------------------------------------------------------------
	//@{
	virtual void setTag (int32_t val);
	virtual int32_t getTag () const { return tag; }

	virtual void beginEdit ();
	virtual void endEdit ();
	bool isEditing () const { return editing > 0; }

	/** get main listener */
	virtual IControlListener* getListener () const { return listener; }
	/** set main listener */
	virtual void setListener (IControlListener* l) { listener = l; }

	/** register a sub listener */
	void registerControlListener (IControlListener* listener);
	/** unregister a sub listener */
	void unregisterControlListener (IControlListener* listener);
	//@}

	//-----------------------------------------------------------------------------
	/// @name Misc
	//-----------------------------------------------------------------------------
	//@{
	virtual void setWheelInc (float val) { wheelInc = val; }
	virtual float getWheelInc () const { return wheelInc; }
	//@}

	// overrides
	void draw (CDrawContext* pContext) override = 0;
	bool isDirty () const override;
	void setDirty (bool val = true) override;

	bool drawFocusOnTop () override;
	bool getFocusPath (CGraphicsPath& outPath) override;

	/** zoom modifier key, per default is the shift key */
	static int32_t kZoomModifier;
	/** default value modifier key, per default is the control key */
	static int32_t kDefaultValueModifier;

	CLASS_METHODS_VIRTUAL(CControl, CView)
protected:
	~CControl () noexcept override = default;
	static int32_t mapVstKeyModifier (int32_t vstModifier);

	using SubListenerDispatcher = DispatchList<IControlListener*>;

	IControlListener* listener;
	SubListenerDispatcher subListeners;
	int32_t  tag;
	float oldValue;
	float defaultValue;
	float value;
	float vmin;
	float vmax;
	float wheelInc;
	int32_t editing;
};

//-----------------------------------------------------------------------------
// IMultiBitmapControl Declaration
//! @brief interface for controls with sub images
//-----------------------------------------------------------------------------
class IMultiBitmapControl
{
public:
	virtual ~IMultiBitmapControl() {}
	virtual void setHeightOfOneImage (const CCoord& height) { heightOfOneImage = height; }
	virtual CCoord getHeightOfOneImage () const { return heightOfOneImage; }

	virtual void setNumSubPixmaps (int32_t numSubPixmaps) { subPixmaps = numSubPixmaps; }
	virtual int32_t getNumSubPixmaps () const { return subPixmaps; }

	virtual void autoComputeHeightOfOneImage ();
protected:
	IMultiBitmapControl () : heightOfOneImage (0), subPixmaps (0) {}
	CCoord heightOfOneImage;
	int32_t subPixmaps;
};

} // VSTGUI
