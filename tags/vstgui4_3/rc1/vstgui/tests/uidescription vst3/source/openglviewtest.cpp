//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 4.3
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2015, Steinberg Media Technologies, All Rights Reserved
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
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#include "openglviewtest.h"
#include "base/source/fobject.h"
#include "base/source/fthread.h"
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

	void platformOpenGLViewCreated () VSTGUI_OVERRIDE_VMETHOD
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
	
	void platformOpenGLViewWillDestroy () VSTGUI_OVERRIDE_VMETHOD
	{
		killThread ();
		removeAllAnimations ();
	}
	
	void platformOpenGLViewSizeChanged () VSTGUI_OVERRIDE_VMETHOD
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
	
	void drawOpenGL (const CRect& updateRect) VSTGUI_OVERRIDE_VMETHOD
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

	PixelFormat* getPixelFormat () VSTGUI_OVERRIDE_VMETHOD
	{
		static PixelFormat pixelFormat;
		pixelFormat.flags = PixelFormat::kAccelerated | PixelFormat::kMultiSample;
		pixelFormat.samples = 4;
		return &pixelFormat;
	}
protected:
	class Thread : public FThread
	{
	public:
		Thread (TestOpenGLView* openGLView)
		: FThread ("OpenGLDrawThread")
		, openGLView (openGLView)
		, cancelDrawLoop (false)
		{
		}
		
		uint32 entry () VSTGUI_OVERRIDE_VMETHOD
		{
			while (cancelDrawLoop == false)
			{
				openGLView->drawOpenGLThreaded ();
				FThreadSleep (16);
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

	virtual void animationStart (CView* view, IdStringPtr name)  VSTGUI_OVERRIDE_VMETHOD {}
	virtual void animationTick (CView* view, IdStringPtr name, float pos) VSTGUI_OVERRIDE_VMETHOD
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
	
	virtual void animationFinished (CView* view, IdStringPtr name, bool wasCanceled) VSTGUI_OVERRIDE_VMETHOD {}

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

	IControlListener* getControlListener (UTF8StringPtr name) VSTGUI_OVERRIDE_VMETHOD
	{
		if (strcmp (name, "threaded") == 0)
			return this;
		return controller->getControlListener (name);
	}
	
	CView* createView (const UIAttributes& attributes, const IUIDescription* description) VSTGUI_OVERRIDE_VMETHOD
	{
		const std::string* name = attributes.getAttributeValue ("custom-view-name");
		if (name && *name == "OpenGLView")
		{
			openGLView = new TestOpenGLView (CRect (0, 0, 0, 0));
			return openGLView;
		}
		return controller->createView (attributes, description);
	}
	
	void valueChanged (CControl* control) VSTGUI_OVERRIDE_VMETHOD
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
