//
//  openglviewtest.cpp
//  uidescription test
//
//  Created by Arne Scheffler on 7/16/11.
//  Copyright 2011 Arne Scheffler. All rights reserved.
//

#include "openglviewtest.h"
#include "base/source/fobject.h"

#if MAC
	#include <OpenGL/gl.h>
	#include <OpenGL/glu.h>
#elif WINDOWS
	#include <GL/gl.h>
	#include <GL/glu.h>
#endif

using namespace Steinberg;
using namespace Steinberg::Vst;

namespace VSTGUI {

//------------------------------------------------------------------------
FUID OpenGLViewTestProcessor::cid (0x40666375, 0x468B4497, 0xAD81E0E9, 0x24776BF3);
FUID OpenGLViewTestController::cid (0xC66B5286, 0x2E334055, 0xB42A6288, 0x9BE0DCE7);

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
OpenGLViewTestProcessor::OpenGLViewTestProcessor ()
{
	setControllerClass (OpenGLViewTestController::cid);
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
class TestOpenGLView : public COpenGLView, public Animation::IAnimationTarget
{
public:
	TestOpenGLView (const CRect& size) : COpenGLView (size) {xRotation = yRotation = zRotation = 0.f;}

	bool attached (CView* parent)
	{
		if (COpenGLView::attached(parent))
		{
			updateViewPort ();
			remember ();
			addAnimation("XRotation", this, new Animation::RepeatTimingFunction (new Animation::LinearTimingFunction (4000), -1, false));
			remember ();
			addAnimation("YRotation", this, new Animation::RepeatTimingFunction (new Animation::LinearTimingFunction (6000), -1, false));
			remember ();
			addAnimation("ZRotation", this, new Animation::RepeatTimingFunction (new Animation::LinearTimingFunction (8000), -1, false));
			return true;
		}
		return false;
	}

	bool removed (CView* parent)
	{
		return COpenGLView::removed (parent);
	}
	
	void setViewSize (const CRect& rect, bool invalid = true)
	{
		COpenGLView::setViewSize (rect, invalid);
		updateViewPort ();
	}
	
	void parentSizeChanged ()
	{
		COpenGLView::parentSizeChanged ();
		updateViewPort ();
	}

	void updateViewPort ()
	{
		if (platformOpenGLView)
		{
			platformOpenGLView->lockContext ();
			platformOpenGLView->makeContextCurrent ();

			glEnable (GL_DEPTH_TEST);
			glEnable (GL_BLEND);
			glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			CRect r (getViewSize ());
			glViewport (0, 0, r.getWidth (), r.getHeight ());

			glClearColor (0, 0, 0, 0);

			platformOpenGLView->unlockContext ();
		}
	}
	
	void drawOpenGL (const CRect& updateRect)
	{
		glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		glMatrixMode (GL_PROJECTION);
		glLoadIdentity ();
		glMatrixMode (GL_MODELVIEW);
		glLoadIdentity ();

		glScaled (0.5, 0.5, 0.2);
		glRotated (xRotation, 1, 0, 0);
		glRotated (yRotation, 0, 1, 0);
		glRotated (zRotation, 0, 0, 1);

		glBegin(GL_TRIANGLES); 

			glColor4f  (0.0f,0.0f,1.0f, 0.5f);
			glVertex3f ( 0.0f, 1.0f, 0.0f);
			glVertex3f (-1.0f,-1.0f, 1.0f);
			glVertex3f ( 1.0f,-1.0f, 1.0f);

			glColor4f  (1.0f,0.0f,0.0f, 0.5f);
			glVertex3f ( 0.0f, 1.0f, 0.0f);
			glVertex3f ( 1.0f,-1.0f, 1.0f);
			glVertex3f ( 1.0f,-1.0f, -1.0f);

			glColor4f  (0.0f,1.0f,0.0f, 0.5f);
			glVertex3f ( 0.0f, 1.0f, 0.0f);
			glVertex3f ( 1.0f,-1.0f, -1.0f);
			glVertex3f (-1.0f,-1.0f, -1.0f);

			glColor4f  (1.0f, 1.0f, 0.0f, 0.5f);
			glVertex3f ( 0.0f, 1.0f, 0.0f);
			glVertex3f (-1.0f,-1.0f,-1.0f);
			glVertex3f (-1.0f,-1.0f, 1.0f);

		glEnd();

		glBegin (GL_QUADS);
			glColor4f (0.5f, 0.5f, 0.5f, 0.9f);
			glVertex3f (-1.f, -1.f, 1.f);
			glVertex3f (1.f, -1.f, 1.f);
			glVertex3f (1.f, -1.f, -1.f);
			glVertex3f (-1.f, -1.f, -1.f);
		glEnd ();
   
		glFlush ();

		platformOpenGLView->swapBuffers ();
	}
protected:
	virtual void animationStart (CView* view, IdStringPtr name) {}
	virtual void animationTick (CView* view, IdStringPtr name, float pos)
	{
		if (strcmp (name, "XRotation") == 0)
			xRotation = pos * 360.f;
		else if (strcmp (name, "ZRotation") == 0)
			zRotation = pos * 360.f;
		else if (strcmp (name, "YRotation") == 0)
			yRotation = pos * 360.f;
		invalid ();
	}
	
	virtual void animationFinished (CView* view, IdStringPtr name, bool wasCanceled) {}

	float xRotation;
	float yRotation;
	float zRotation;
};

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
Steinberg::tresult PLUGIN_API OpenGLViewTestController::initialize (Steinberg::FUnknown* context)
{
	tresult res = UIDescriptionBaseController::initialize (context);
	if (res == kResultTrue)
	{
	}
	return res;
}

//------------------------------------------------------------------------
Steinberg::IPlugView* PLUGIN_API OpenGLViewTestController::createView (Steinberg::FIDString name)
{
	if (strcmp (name, ViewType::kEditor) == 0)
	{
		return new VST3Editor (this, "view", "OpenGLViewTest.uidesc");
	}
	return 0;
}

//------------------------------------------------------------------------
CView* OpenGLViewTestController::createCustomView (UTF8StringPtr name, const UIAttributes& attributes, IUIDescription* description, VST3Editor* editor)
{
	if (strcmp (name, "OpenGLView") == 0)
	{
		return new TestOpenGLView (CRect (0, 0, 0, 0));
	}
	return 0;
}

} // namespace
