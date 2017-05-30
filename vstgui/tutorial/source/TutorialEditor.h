// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __TutorialEditor__
#define __TutorialEditor__

#include "../../plugin-bindings/aeffguieditor.h"
#include "TutorialParameters.h"

class TutorialEditor : public AEffGUIEditor, public CControlListener
{
public:
	TutorialEditor (void*);
	
	// from AEffGUIEditor
	bool open (void* ptr);
	void close ();
	void setParameter (VstInt32 index, float value);

	// from CControlListener
	void valueChanged (CControl* pControl);

protected:
	CControl* controls[kNumParameters];
};

#endif // __TutorialEditor__

