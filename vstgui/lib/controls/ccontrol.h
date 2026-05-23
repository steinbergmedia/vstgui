// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../cview.h"
#include "../cbitmap.h"
#include "../ifocusdrawing.h"

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
	CControl (const CRect& size, IControlListener* listener = nullptr, int32_t tag = 0,
			  const SharedPointer<CBitmap>& background = {});
	CControl (const CControl& c);

	//-----------------------------------------------------------------------------
	/// @name Value Methods
	//-----------------------------------------------------------------------------
	//@{
	virtual bool setValue (float val);
	virtual float getValue () const;

	virtual bool setValueNormalized (float val);
	virtual float getValueNormalized () const;

	virtual void setMin (float val);
	virtual float getMin () const;
	virtual void setMax (float val);
	virtual float getMax () const;
	float getRange () const { return getMax () - getMin (); }

	virtual void setOldValue (float val);
	virtual	float getOldValue () const;
	virtual void setDefaultValue (float val);
	virtual	float getDefaultValue () const;

	virtual void bounceValue ();
	
	/** notifies listener and dependent objects */
	virtual void valueChanged ();
	//@}

	//-----------------------------------------------------------------------------
	/// @name Editing Methods
	//-----------------------------------------------------------------------------
	//@{
	virtual void setTag (int32_t val);
	virtual int32_t getTag () const;

	virtual void beginEdit ();
	virtual void endEdit ();
	bool isEditing () const;

	/** get main listener */
	virtual IControlListener* getListener () const;
	/** set main listener */
	virtual void setListener (IControlListener* l);

	/** register a sub listener */
	void registerControlListener (IControlListener* listener);
	/** unregister a sub listener */
	void unregisterControlListener (IControlListener* listener);
	//@}

	//-----------------------------------------------------------------------------
	/// @name Misc
	//-----------------------------------------------------------------------------
	//@{
	virtual void setWheelInc (float val);
	virtual float getWheelInc () const;
	//@}

	// overrides
	bool removed (CViewContainer& parent) override;
	bool attached (CViewContainer& parent) override;
	void draw (CDrawContext& context) override = 0;

	bool drawFocusOnTop () override;
	bool getFocusPath (CGraphicsPath& outPath, CCoord focusLineWidth) override;

	using CheckDefaultValueEventFuncT = bool (*) (CControl*, MouseDownEvent&);
	/** Function to check if a mouse down event should reset the value to its default value for a
	 *control. Per default this checks for a left mouse down button and the control modifier key. */
	static CheckDefaultValueEventFuncT CheckDefaultValueEventFunc;

	/** zoom modifier key, per default is the shift key */
	inline static int32_t kZoomModifier = kShift;

	CLASS_METHODS_VIRTUAL(CControl, CView)
protected:
	~CControl () noexcept override;

private:
	struct Impl;
	std::unique_ptr<Impl> impl;
};

//-----------------------------------------------------------------------------
// CMouseWheelEditingSupport Declaration
//! @brief Helper class for mouse wheel editing
//-----------------------------------------------------------------------------
class CMouseWheelEditingSupport
{
protected:
	void invalidMouseWheelEditTimer (CControl& control);
	void onMouseWheelEditing (CControl& control);

private:
	SharedPointer<CBaseObject> endEditTimer {nullptr};
};

} // VSTGUI
