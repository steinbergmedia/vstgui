// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#include "hiviewtextedit.h"
#include "hiviewframe.h"
#include "../cfontmac.h"
#include "../macstring.h"

#if MAC_CARBON

#include "../../iplatformopenglview.h"
#include "../../iplatformviewlayer.h"

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
		selection.selEnd = static_cast<SInt16> (strlen (text));
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
		fontStyle.size = static_cast<SInt16> (fontID->getSize ());
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
HIViewTextEdit::~HIViewTextEdit () noexcept
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
bool HIViewTextEdit::setText (const UTF8String& text)
{
	if (platformControl)
	{
		CFStringRef textString = fromUTF8String<CFStringRef> (text);
		SetControlData (platformControl, kControlEditTextPart, kControlEditTextCFStringTag, sizeof (CFStringRef), &textString);
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
UTF8String HIViewTextEdit::getText ()
{
	if (platformControl)
	{
		CFStringRef cfstr;
		if (GetControlData (platformControl, kControlEditTextPart, kControlEditTextCFStringTag, sizeof cfstr, (void*)&cfstr, NULL) == noErr)
		{
			freeText ();
			CFIndex textSize = CFStringGetMaximumSizeForEncoding (CFStringGetLength (cfstr), kCFStringEncodingUTF8);
			text = static_cast<UTF8StringBuffer> (std::malloc (static_cast<size_t> (textSize)));
			
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
		r.origin.x = static_cast<CGFloat> (rect.left);
		r.origin.y = static_cast<CGFloat> (rect.top);
		r.size.width = static_cast<CGFloat> (rect.getWidth ());
		r.size.height = static_cast<CGFloat> (rect.getHeight ());
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
