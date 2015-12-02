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

#ifndef __iuidescription__
#define __iuidescription__

#include "../lib/vstguifwd.h"
#include "../lib/cfont.h"
#include <string>
#include <list>

namespace VSTGUI {

class IController;
class IViewFactory;

//-----------------------------------------------------------------------------
class IUIDescription
{
public:
	virtual ~IUIDescription () {}

	virtual CView* createView (UTF8StringPtr name, IController* controller) const = 0;

	virtual CBitmap* getBitmap (UTF8StringPtr name) const = 0;
	virtual CFontRef getFont (UTF8StringPtr name) const = 0;
	virtual bool getColor (UTF8StringPtr name, CColor& color) const = 0;
	virtual CGradient* getGradient (UTF8StringPtr name) const = 0;
	virtual int32_t getTagForName (UTF8StringPtr name) const = 0;
	virtual IControlListener* getControlListener (UTF8StringPtr name) const = 0;
	virtual IController* getController () const = 0;

	virtual UTF8StringPtr lookupColorName (const CColor& color) const = 0;
	virtual UTF8StringPtr lookupFontName (const CFontRef font) const = 0;
	virtual UTF8StringPtr lookupBitmapName (const CBitmap* bitmap) const = 0;
	virtual UTF8StringPtr lookupGradientName (const CGradient* gradient) const = 0;
	virtual UTF8StringPtr lookupControlTagName (const int32_t tag) const = 0;

	virtual bool getVariable (UTF8StringPtr name, double& value) const = 0;
	virtual bool getVariable (UTF8StringPtr name, std::string& value) const = 0;

	virtual void collectTemplateViewNames (std::list<const std::string*>& names) const = 0;
	virtual void collectColorNames (std::list<const std::string*>& names) const = 0;
	virtual void collectFontNames (std::list<const std::string*>& names) const = 0;
	virtual void collectBitmapNames (std::list<const std::string*>& names) const = 0;
	virtual void collectGradientNames (std::list<const std::string*>& names) const = 0;
	virtual void collectControlTagNames (std::list<const std::string*>& names) const = 0;

	virtual const IViewFactory* getViewFactory () const = 0;

	static IdStringPtr kCustomViewName;
};


} // namespace VSTGUI

#endif // __iuidescription__
