// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __copenglview__
#define __copenglview__

#include "cview.h"
#include "platform/iplatformopenglview.h"

#if VSTGUI_OPENGL_SUPPORT

namespace VSTGUI {

/*
TODO: Documentation

	To setup OpenGL for a normal 2D matrix use this to setup the OpenGL context:

	CRect r (getViewSize ());
	glViewport (0, 0, r.getWidth (), r.getHeight ());
	gluOrtho2D (r.left, r.right, r.bottom, r.top);
	glTranslated (r.left, r.top, 0);

*/
//-----------------------------------------------------------------------------
// COpenGLView Declaration
/// @brief a subview which uses OpenGL for drawing
/// @ingroup new_in_4_1
//-----------------------------------------------------------------------------
class COpenGLView : public CView, public IOpenGLView
{
public:
	explicit COpenGLView (const CRect& size);
	~COpenGLView () noexcept override;

	// IOpenGLView	
	void drawOpenGL (const CRect& updateRect) override = 0;	///< will be called when the view was marked invalid or the view was resized
	void reshape () override;

	// CView
	void setViewSize (const CRect& rect, bool invalid = true) override;
	void parentSizeChanged () override;
	bool removed (CView* parent) override;
	bool attached (CView* parent) override;
	void invalidRect (const CRect& rect) override;
	void setVisible (bool state) override;

	CLASS_METHODS_NOCOPY (COpenGLView, CView)
protected:
	//-----------------------------------------------------------------------------
	/// @name COpenGLView Methods
	//-----------------------------------------------------------------------------
	//@{
	virtual void platformOpenGLViewCreated () {}			///< will be called after the platform opengl view was created
	virtual void platformOpenGLViewWillDestroy () {}		///< will be called before the platform opengl view will be destroyed
	virtual void platformOpenGLViewSizeChanged () {}		///< will be called whenever the platform opengl view size has changed
	virtual PixelFormat* getPixelFormat () { return 0; }	///< subclasses should return a pixelformat here if they don't want to use the default one
	IPlatformOpenGLView* getPlatformOpenGLView () const { return platformOpenGLView; }
	//@}

private:
	void updatePlatformOpenGLViewSize ();
	bool createPlatformOpenGLView ();
	bool destroyPlatformOpenGLView ();

	SharedPointer<IPlatformOpenGLView> platformOpenGLView;
};

} // namespace

#endif // VSTGUI_OPENGL_SUPPORT
#endif // __copenglview__
