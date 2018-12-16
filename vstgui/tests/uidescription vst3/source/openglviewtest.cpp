// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "openglviewtest.h"
#include "base/source/fobject.h"
#include "base/thread/include/fthread.h"
#include "vstgui/uidescription/delegationcontroller.h"
#include "vstgui/uidescription/uiattributes.h"

#if MAC
	#include <OpenGL/gl.h>
	#include <OpenGL/glu.h>
#elif WINDOWS
	#include <windows.h>
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
	TestOpenGLView (const CRect& size) : COpenGLView (size), useThread (false), thread (0)
	{
	}

	void killThread ()
	{
		if (thread)
		{
			thread->stop ();
			thread->waitDead (-1);
			delete thread;
			thread = 0;
		}
	}
	
	void setThreaded (bool state)
	{
		if (isAttached ())
		{
			xRotation = yRotation = zRotation = 0.f;
			if (state == true && thread == 0)
			{
				thread = new Thread (this);
				thread->run ();
			}
			else if (state == false)
			{
				killThread ();
				remember ();
				addAnimation("XRotation", this, new Animation::RepeatTimingFunction (new Animation::LinearTimingFunction (4000), -1, false));
				remember ();
				addAnimation("YRotation", this, new Animation::RepeatTimingFunction (new Animation::LinearTimingFunction (6000), -1, false));
				remember ();
				addAnimation("ZRotation", this, new Animation::RepeatTimingFunction (new Animation::LinearTimingFunction (8000), -1, false));
			}
		}
		useThread = state;
	}

	void platformOpenGLViewCreated () override
	{
		getPlatformOpenGLView ()->lockContext ();
		getPlatformOpenGLView ()->makeContextCurrent ();

		glEnable (GL_DEPTH_TEST);
		glEnable (GL_BLEND);
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glClearColor (0, 0, 0, 0);

		getPlatformOpenGLView ()->unlockContext ();

		setThreaded (useThread);
	}
	
	void platformOpenGLViewWillDestroy () override
	{
		killThread ();
		removeAllAnimations ();
	}
	
	void platformOpenGLViewSizeChanged () override
	{
		if (getPlatformOpenGLView ())
		{
			getPlatformOpenGLView ()->lockContext ();
			getPlatformOpenGLView ()->makeContextCurrent ();

			CRect r (getViewSize ());
			glViewport (0, 0, static_cast<GLsizei> (r.getWidth ()), static_cast<GLsizei> (r.getHeight ()));

			getPlatformOpenGLView ()->unlockContext ();
		}
	}

	void drawOpenGLThreaded ()
	{
		if (getPlatformOpenGLView ())
		{
			xRotation += 1.f;
			if (xRotation >= 360.f)
				xRotation = 0.f;
			yRotation += 1.f;
			if (yRotation >= 360.f)
				yRotation = 0.f;
			zRotation += 1.f;
			if (zRotation >= 360.f)
				zRotation = 0.f;
			getPlatformOpenGLView ()->lockContext ();
			getPlatformOpenGLView ()->makeContextCurrent ();

			drawOpenGL (getViewSize ());

			getPlatformOpenGLView ()->unlockContext ();
		}
	}
	
	void drawOpenGL (const CRect& updateRect) override
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

		getPlatformOpenGLView ()->swapBuffers ();
	}

	PixelFormat* getPixelFormat () override
	{
		static PixelFormat pixelFormat;
		pixelFormat.flags = PixelFormat::kMultiSample;
		pixelFormat.samples = 4;
		return &pixelFormat;
	}
protected:
	class Thread : public Base::Thread::FThread
	{
	public:
		Thread (TestOpenGLView* openGLView)
		: FThread ("OpenGLDrawThread")
		, openGLView (openGLView)
		, cancelDrawLoop (false)
		{
		}
		
		uint32 entry () override
		{
			while (cancelDrawLoop == false)
			{
				openGLView->drawOpenGLThreaded ();
				Base::Thread::FThreadSleep (16);
			}
			return 0;
		}

		void stop ()
		{
			cancelDrawLoop = true;
		}

	protected:
		TestOpenGLView* openGLView;
		volatile bool cancelDrawLoop;
	};

	virtual void animationStart (CView* view, IdStringPtr name)  override {}
	virtual void animationTick (CView* view, IdStringPtr name, float pos) override
	{
		if (thread == 0)
		{
			if (strcmp (name, "XRotation") == 0)
				xRotation = pos * 360.f;
			else if (strcmp (name, "ZRotation") == 0)
				zRotation = pos * 360.f;
			else if (strcmp (name, "YRotation") == 0)
				yRotation = pos * 360.f;
			invalid ();
		}
	}
	
	virtual void animationFinished (CView* view, IdStringPtr name, bool wasCanceled) override {}

	float xRotation;
	float yRotation;
	float zRotation;
	
	bool useThread;
	Thread* thread;
};

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
class OpenGLViewController : public DelegationController
{
public:
	OpenGLViewController (IController* baseController)
	: DelegationController (baseController)
	, openGLView (0)
	{}

	IControlListener* getControlListener (UTF8StringPtr name) override
	{
		if (strcmp (name, "threaded") == 0)
			return this;
		return controller->getControlListener (name);
	}
	
	CView* createView (const UIAttributes& attributes, const IUIDescription* description) override
	{
		const std::string* name = attributes.getAttributeValue ("custom-view-name");
		if (name && *name == "OpenGLView")
		{
			openGLView = new TestOpenGLView (CRect (0, 0, 0, 0));
			return openGLView;
		}
		return controller->createView (attributes, description);
	}
	
	void valueChanged (CControl* control) override
	{
		if (openGLView)
			openGLView->setThreaded (control->getValue () == control->getMax ());
	}
	
protected:
	TestOpenGLView* openGLView;
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
IController* OpenGLViewTestController::createSubController (const char* name, const IUIDescription* description, VST3Editor* editor)
{
	if (strcmp (name, "OpenGLViewController") == 0)
		return new OpenGLViewController (editor);
	return 0;
}

} // namespace
