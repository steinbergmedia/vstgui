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

#include "hiviewtextedit.h"
#include "hiviewframe.h"
#include "../cfontmac.h"

#if MAC_CARBON

#pragma GCC diagnostic ignored "-Wdeprecated-declarations" // we know that we use deprecated functions from Carbon, so we don't want to be warned

namespace VSTGUI {

//-----------------------------------------------------------------------------
HIViewTextEdit::HIViewTextEdit (HIViewRef parent, IPlatformTextEditCallback* textEdit)
: IPlatformTextEdit (textEdit)
, eventHandler (0)
, text (0)
{
	extern bool hiToolboxAllowFocusChange;
	bool oldState = hiToolboxAllowFocusChange;
	hiToolboxAllowFocusChange = false;
	
	WindowRef window = HIViewGetWindow (parent);
	CRect rect = textEdit->platformGetSize ();
	CFontRef fontID = textEdit->platformGetFont ();

	if (!isWindowComposited (window))
	{
		HIRect hiRect;
		HIViewGetFrame (parent, &hiRect);
		rect.offset ((CCoord)hiRect.origin.x, (CCoord)hiRect.origin.y);
	}
	Rect r;
	r.left   = (short)rect.left;// + 2;
	r.right  = (short)rect.right;// - 4;
	r.top    = (short)rect.top;// + 2;
	r.bottom = (short)rect.bottom;// - 4;
	if (rect.getHeight () > fontID->getSize ())
	{
		r.top = (short)(rect.top + rect.getHeight () / 2 - fontID->getSize () / 2 + 1);
		r.bottom = (short)(r.top + fontID->getSize ());
	}
	HIViewRef textControl = 0;
	UTF8StringPtr text = textEdit->platformGetText ();
	if (CreateEditUnicodeTextControl (NULL, &r, NULL, false, NULL, &textControl) == noErr)
	{
		HIViewAddSubview (parent, textControl);
		EventTypeSpec eventTypes[] = {
			{ kEventClassWindow, kEventWindowDeactivated },
			{ kEventClassKeyboard, kEventRawKeyDown },
			{ kEventClassKeyboard, kEventRawKeyRepeat },
			{ kEventClassControl, kEventControlDraw },
			{ kEventClassTextField, kEventTextDidChange }
		};
		InstallControlEventHandler (textControl, CarbonEventsTextControlProc, GetEventTypeCount (eventTypes), eventTypes, this, &eventHandler);
		platformControl = textControl;

		setText (text);

		ControlEditTextSelectionRec selection;
		selection.selStart = 0;
		selection.selEnd = strlen (text);
		SetControlData (platformControl, kControlEditTextPart, kControlEditTextSelectionTag, sizeof (ControlEditTextSelectionRec), &selection);

		Boolean singleLineStyle = true;
		SetControlData (textControl, kControlEditTextPart, kControlEditTextSingleLineTag, sizeof (Boolean), &singleLineStyle);
		ControlFontStyleRec fontStyle;
		memset (&fontStyle, 0, sizeof (fontStyle));
		fontStyle.flags = kControlUseJustMask | kControlUseSizeMask | kControlUseFontMask;
		switch (textEdit->platformGetHoriTxtAlign ())
		{
			case kLeftText: fontStyle.just = teFlushLeft; break;
			case kRightText: fontStyle.just = teFlushRight; break;
			default: fontStyle.just = teCenter; break;
		}
		fontStyle.size = fontID->getSize ();
	#if MAC_OS_X_VERSION_MAX_ALLOWED <= MAC_OS_X_VERSION_10_6
		Str255 fontName;
		CopyCStringToPascal (fontID->getName (), fontName);
		GetFNum (fontName, &fontStyle.font);
	#else
		#warning Mac OS X 10.7 Carbon incompatibility. It looks like it's not possible to set the font family for a text control anymore
	#endif
		SetControlData (textControl, kControlEditTextPart, kControlFontStyleTag, sizeof (fontStyle), &fontStyle);
		HIViewSetVisible (textControl, true);
		HIViewAdvanceFocus (textControl, 0);
		SetKeyboardFocus (window, textControl, kControlEditTextPart);
		SetUserFocusWindow (window);
	}
	hiToolboxAllowFocusChange = oldState;
}

//-----------------------------------------------------------------------------
HIViewTextEdit::~HIViewTextEdit ()
{
	if (eventHandler)
		RemoveEventHandler (eventHandler);
	if (platformControl)
	{
		HIViewSetVisible (platformControl, false);
		HIViewRemoveFromSuperview (platformControl);
		SetUserFocusWindow (kUserFocusAuto);
		CFRelease (platformControl);
		SetThemeCursor (kThemeArrowCursor);
	}
	freeText ();
}

//-----------------------------------------------------------------------------
void HIViewTextEdit::freeText ()
{
	if (text)
		std::free (text);
	text = 0;
}

//-----------------------------------------------------------------------------
bool HIViewTextEdit::setText (UTF8StringPtr text)
{
	if (platformControl)
	{
		CFStringRef textString = CFStringCreateWithCString (NULL, text, kCFStringEncodingUTF8);
		if (textString)
		{
			SetControlData (platformControl, kControlEditTextPart, kControlEditTextCFStringTag, sizeof (CFStringRef), &textString);
			CFRelease (textString);
		}
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
UTF8StringPtr HIViewTextEdit::getText ()
{
	if (platformControl)
	{
		CFStringRef cfstr;
		if (GetControlData (platformControl, kControlEditTextPart, kControlEditTextCFStringTag, sizeof cfstr, (void*)&cfstr, NULL) == noErr)
		{
			freeText ();
			CFIndex textSize = CFStringGetMaximumSizeForEncoding (CFStringGetLength (cfstr), kCFStringEncodingUTF8);
			text = (UTF8StringBuffer)std::malloc (textSize);
			
			CFStringGetCString (cfstr, text, textSize, kCFStringEncodingUTF8);
			CFRelease (cfstr);
			return text;
		}
	}
	return "";
}

//-----------------------------------------------------------------------------
bool HIViewTextEdit::updateSize ()
{
	if (platformControl)
	{
		CRect rect = textEdit->platformGetSize ();
		if (!isWindowComposited (HIViewGetWindow (platformControl)))
		{
			HIRect hiRect;
			HIViewGetFrame (HIViewGetSuperview (platformControl), &hiRect);
			rect.offset ((CCoord)hiRect.origin.x, (CCoord)hiRect.origin.y);
		}
		HIRect r;
		r.origin.x = rect.left;
		r.origin.y = rect.top;
		r.size.width = rect.getWidth ();
		r.size.height = rect.getHeight ();
		if (HIViewSetFrame (platformControl, &r) == noErr)
			return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
pascal OSStatus HIViewTextEdit::CarbonEventsTextControlProc (EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
{
	OSStatus result = eventNotHandledErr;
	UInt32 eventClass = GetEventClass (inEvent);
	UInt32 eventKind = GetEventKind (inEvent);
	HIViewTextEdit* textEdit = (HIViewTextEdit*)inUserData;

	switch (eventClass)
	{
		case kEventClassKeyboard:
		{
			switch (eventKind)
			{
				case kEventRawKeyDown:
				case kEventRawKeyRepeat:
				{
					char macCharCode;
					UInt32 keyCode;
					UInt32 modifiers;
					GetEventParameter (inEvent, kEventParamKeyMacCharCodes, typeChar, NULL, sizeof (char), NULL, &macCharCode);
					GetEventParameter (inEvent, kEventParamKeyCode, typeUInt32, NULL, sizeof (UInt32), NULL, &keyCode);
					GetEventParameter (inEvent, kEventParamKeyModifiers, typeUInt32, NULL, sizeof (UInt32), NULL, &modifiers);
					if (macCharCode == 13 || macCharCode == 3 || macCharCode == 27)
					{
						if (macCharCode == 27)
						{
							textEdit->setText (textEdit->textEdit->platformGetText ());
						}
						textEdit->textEdit->platformLooseFocus (macCharCode == 27 ? false : true);

						result = noErr;
					}
					break;
				}
			}
			break;
		}
		case kEventClassTextField:
		{
			switch (eventKind)
			{
				case kEventTextDidChange:
				{
					textEdit->textEdit->platformTextDidChange ();
					break;
				}
			}
			break;
		}
		case kEventClassControl:
		{
			switch (eventKind)
			{
			#if 0 // TODO: make this work
			#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4)
				case kEventControlDraw:
				{
					CGContextRef cgContext;
					if (GetEventParameter (inEvent, kEventParamCGContextRef, typeCGContextRef, NULL, sizeof (cgContext), NULL, &cgContext) == noErr)
					{
						#if MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_4
						if (HIRectConvert)
						#endif
						{
							CRect viewSize = textEdit->getViewSize (viewSize);
							viewSize.extend (10, 10);
							CViewContainer* container = (CViewContainer*)textEdit->getParentView ();
							while (!container->isTypeOf ("CScrollContainer"))
							{
								CRect containerSize = container->getViewSize (containerSize);
								viewSize.offset (containerSize.left, containerSize.top);
								if (container == container->getParentView () || container->getParentView () == 0)
									break;
								container = ((CViewContainer*)container->getParentView ());
							}
							viewSize = container->getVisibleSize (viewSize);
							CPoint cp (viewSize.left, viewSize.top);
							container->localToFrame (cp);
							viewSize.offset (-viewSize.left, -viewSize.top);
							viewSize.offset (cp.x, cp.y);
							CGRect cgViewSize = CGRectMake (viewSize.left, viewSize.top, viewSize.getWidth (), viewSize.getHeight ());
							HIRectConvert (&cgViewSize, kHICoordSpaceView, (HIViewRef)textEdit->getFrame ()->getPlatformControl (), kHICoordSpaceView, textEdit->platformControl);
							CGContextClipToRect (cgContext, cgViewSize);
							CGAffineTransform ctm = CGContextGetCTM (cgContext);
						}
						result = CallNextEventHandler (inHandlerCallRef, inEvent);
					}
					break;
				}
			#endif
			#endif
			}
			break;
		}
		case kEventClassWindow:
		{
			WindowRef window;
			if (GetEventParameter (inEvent, kEventParamDirectObject, typeWindowRef, NULL, sizeof (WindowRef), NULL, &window) != noErr)
				break;
			switch (eventKind)
			{
				case kEventWindowDeactivated:
				{
//					result = CallNextEventHandler (inHandlerCallRef, inEvent);
//					ClearKeyboardFocus (window);

					textEdit->textEdit->platformLooseFocus (false);

					break;
				}
			}
			break;
		}
	}
	return result;
}

} // namespace

#pragma GCC diagnostic warning "-Wdeprecated-declarations" // we know that we use deprecated functions from Carbon, so we don't want to be warned

#endif
