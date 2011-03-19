//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework not only for VST plugins : 
//
// Version 4.0
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2010, Steinberg Media Technologies, All Rights Reserved
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
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A  PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#if VSTGUI_LIVE_EDITING

#include "uiviewinspector.h"
#include "uiselection.h"
#include "uiviewfactory.h"
#include "uifontchooserpanel.h"
#include "editingcolordefs.h"
#include "../lib/cscrollview.h"
#include "../lib/ctabview.h"
#include "../lib/cdatabrowser.h"
#include "../lib/vstkeycode.h"
#include "../lib/cgraphicspath.h"
#include "../lib/cfont.h"
#include "../lib/ifocusdrawing.h"
#include <set>
#include <vector>
#include <algorithm>
#include <sstream>

namespace VSTGUI {

//-----------------------------------------------------------------------------
class InspectorTabButton : public COnOffButton
//-----------------------------------------------------------------------------
{
public:
	InspectorTabButton (const CRect &size, UTF8StringPtr inName, int32_t tabPosition = 0)
	: COnOffButton (size, 0, -1, 0)
	, name (0)
	, tabPosition (tabPosition)
	{
		if (inName)
		{
			name = (UTF8StringBuffer)malloc (strlen (inName) + 1);
			strcpy (name, inName);
		}
		backgroundColor = kTransparentCColor; // uidWindowBackgroundColor;
		activeTextColor = kWhiteCColor;
		inactiveTextColor = kGreyCColor;
		textFont = kSystemFont; textFont->remember ();
	}

	virtual ~InspectorTabButton ()
	{
		if (textFont)
			textFont->forget ();
		if (name)
			free (name);
	}	

	bool drawFocusOnTop ()
	{
		return false;
	}
	
	bool getFocusPath (CGraphicsPath& outPath)
	{
		CRect r = getViewSize ();
		r.inset (2, 5);
		outPath.addRect (r);
		return true;
	}
	
	virtual void draw (CDrawContext *pContext)
	{
		pContext->setDrawMode (kAliasing);
		pContext->setFillColor (backgroundColor);
		CRect r (size);
		if (tabPosition < 0)
		{
			CRect r2 (r);
			r.left += r.getHeight () / 2;

			r2.right = r2.left + r2.getHeight ();
			pContext->drawArc (r2, 0, 90, kDrawFilled);
			r2.top += r.getHeight () / 2;
			r2.right = r.left+1;
			pContext->drawRect (r2, kDrawFilled);
		}
		else if (tabPosition > 0)
		{
			CRect r2 (r);
			r.right -= r.getHeight () / 2;

			r2.left = r2.right - r2.getHeight ();
			pContext->drawArc (r2, 270, 360, kDrawFilled);
			r2.top += r.getHeight () / 2;
			r2.left = r.right-1;
			pContext->drawRect (r2, kDrawFilled);
		}
		pContext->drawRect (r, kDrawFilled);
		if (name)
		{
			pContext->setFont (textFont);
			pContext->setFontColor (value ? activeTextColor : inactiveTextColor);
			pContext->drawString (name, size);
		}
		CColor lineColor (backgroundColor);
		lineColor.alpha += 50;
		pContext->setFrameColor (lineColor);
		pContext->setLineWidth (1);
		pContext->setLineStyle (kLineSolid);
		pContext->moveTo (CPoint (size.left, size.bottom));
		pContext->lineTo (CPoint (size.right, size.bottom));
	}

	CMouseEventResult onMouseDown (CPoint &where, const CButtonState& button)
	{
		beginEdit ();
		value = ((int32_t)value) ? 0.f : 1.f;
		valueChanged ();
		endEdit ();
		return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
	}

	virtual void onDragEnter (CDragContainer* drag, const CPoint& where)
	{
		if (value == 0.f)
		{
			value = 1.f;
			valueChanged ();
		}
	}

	CLASS_METHODS (InspectorTabButton, COnOffButton)
protected:
	UTF8StringBuffer name;
	CFontRef textFont;
	CColor activeTextColor;
	CColor inactiveTextColor;
	CColor backgroundColor;
	int32_t tabPosition;
};


//-----------------------------------------------------------------------------
class AttributeChangeAction : public IActionOperation, protected std::map<CView*, std::string>
//-----------------------------------------------------------------------------
{
public:
	AttributeChangeAction (UIDescription* desc, UISelection* selection, const std::string& attrName, const std::string& attrValue)
	: desc (desc)
	, attrName (attrName)
	, attrValue (attrValue)
	{
		UIViewFactory* viewFactory = dynamic_cast<UIViewFactory*> (desc->getViewFactory ());
		std::string attrOldValue;
		FOREACH_IN_SELECTION(selection, view)
			viewFactory->getAttributeValue (view, attrName, attrOldValue, desc);
			insert (std::make_pair (view, attrOldValue));
			view->remember ();
		FOREACH_IN_SELECTION_END
		name = "'" + attrName + "' change";
	}

	~AttributeChangeAction ()
	{
		const_iterator it = begin ();
		while (it != end ())
		{
			(*it).first->forget ();
			it++;
		}
	}

	UTF8StringPtr getName ()
	{
		return name.c_str ();
	}
	
	void perform ()
	{
		UIViewFactory* viewFactory = dynamic_cast<UIViewFactory*> (desc->getViewFactory ());
		UIAttributes attr;
		attr.setAttribute (attrName.c_str (), attrValue.c_str ());
		const_iterator it = begin ();
		while (it != end ())
		{
			(*it).first->invalid ();	// we need to invalid before changing anything as the size may change
			viewFactory->applyAttributeValues ((*it).first, attr, desc);
			(*it).first->invalid ();	// and afterwards also
			it++;
		}
	}
	
	void undo ()
	{
		UIViewFactory* viewFactory = dynamic_cast<UIViewFactory*> (desc->getViewFactory ());
		const_iterator it = begin ();
		while (it != end ())
		{
			UIAttributes attr;
			attr.setAttribute (attrName.c_str (), (*it).second.c_str ());
			(*it).first->invalid ();	// we need to invalid before changing anything as the size may change
			viewFactory->applyAttributeValues ((*it).first, attr, desc);
			(*it).first->invalid ();	// and afterwards also
			it++;
		}
	}
protected:
	UIDescription* desc;
	std::string attrName;
	std::string attrValue;
	std::string name;
};

//-----------------------------------------------------------------------------
class FocusOptionMenu : public COptionMenu
//-----------------------------------------------------------------------------
{
public:
	FocusOptionMenu (const CRect& size, CControlListener* listener, int32_t tag, CBitmap* background = 0, CBitmap* bgWhenClick = 0, const int32_t style = 0)
	: COptionMenu (size, listener, tag, background, bgWhenClick, style) {}
	
	void takeFocus ()
	{
		origBackgroundColor = backColor;
		backColor.alpha = 255;
	}
	
	void looseFocus ()
	{
		backColor = origBackgroundColor;
	}

protected:
	CColor origBackgroundColor;
};

//-----------------------------------------------------------------------------
class SimpleBooleanButton : public CParamDisplay
//-----------------------------------------------------------------------------
{
public:
	SimpleBooleanButton (const CRect& size, CControlListener* listener)
	: CParamDisplay (size)
	{
		setListener (listener);
		setValueToStringProc (booleanStringConvert, 0);
		setWantsFocus (true);
	}

	static bool booleanStringConvert (float value, char string[256], void* userData)
	{
		if (value == 0)
			strcpy (string, "false");
		else
			strcpy (string, "true");
		return true;
	}
	
	CMouseEventResult onMouseDown (CPoint &where, const CButtonState& buttons)
	{
		value = value == 0.f ? 1.f : 0.f;
		beginEdit ();
		valueChanged ();
		endEdit ();
		invalid ();
		return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
	}

	void takeFocus ()
	{
		origBackgroundColor = backColor;
		backColor.alpha = 255;
	}
	
	void looseFocus ()
	{
		backColor = origBackgroundColor;
	}
	
	int32_t onKeyDown (VstKeyCode& keyCode)
	{
		if (keyCode.virt == VKEY_RETURN && keyCode.modifier == 0)
		{
			value = ((int32_t)value) ? 0.f : 1.f;
			invalid ();
			beginEdit ();
			valueChanged ();
			endEdit ();
			return 1;
		}
		return -1;
	}
	CColor origBackgroundColor;
};

//-----------------------------------------------------------------------------
class BrowserDelegateBase : public CBaseObject, public IDataBrowser
//-----------------------------------------------------------------------------
{
public:
	BrowserDelegateBase (UIDescription* desc, IActionOperator* actionOperator)
	: desc (desc), actionOperator (actionOperator), mouseRow (-1), path (0), db (0)
	{
		desc->addDependency (this);
	}
	
	~BrowserDelegateBase ()
	{
		desc->removeDependency (this);
		if (path)
			path->forget ();
	}
	
	void dbAttached (CDataBrowser* browser)
	{
		db = browser;
	}
	
	void dbRemoved (CDataBrowser* browser)
	{
		db = 0;
	}

	virtual void getNames (std::list<const std::string*>& _names) = 0;

	void updateNames ()
	{
		std::list<const std::string*> _names;
		getNames (_names);
		_names.sort (std__stringCompare);
		names.clear ();
		names.insert (names.begin (), _names.begin (), _names.end ());
		if (db)
			db->recalculateLayout (true);
	}

	int32_t dbGetNumRows (CDataBrowser* browser)
	{
		return (int32_t)names.size () + 1;
	}
	
	int32_t dbGetNumColumns (CDataBrowser* browser)
	{
		return 3;
	}
	
	bool dbGetColumnDescription (int32_t index, CCoord& minWidth, CCoord& maxWidth, CDataBrowser* browser)
	{
		return false;
	}
	
	CCoord dbGetCurrentColumnWidth (int32_t index, CDataBrowser* browser)
	{
		if (index == 2)
			return 20;
		return (browser->getWidth () - 40) / 2;
	}
	
	void dbSetCurrentColumnWidth (int32_t index, const CCoord& width, CDataBrowser* browser)
	{
	}
	
	CCoord dbGetRowHeight (CDataBrowser* browser)
	{
		return 20;
	}
	
	bool dbGetLineWidthAndColor (CCoord& width, CColor& color, CDataBrowser* browser)
	{
		width = 1;
		color = uidDataBrowserLineColor;
		return true;
	}

	void dbDrawHeader (CDrawContext* context, const CRect& size, int32_t column, int32_t flags, CDataBrowser* browser)
	{
		CRect r (size);
		r.inset (0, 3);
		context->setDrawMode (kAliasing);
		context->setFillColor (uidDataBrowserLineColor);
		context->drawRect (r, kDrawFilled);
		if (headerTitles.size () > (size_t)column)
		{
			context->setFont (kSystemFont, 10);
			context->setFontColor (kWhiteCColor);
			context->drawString (headerTitles[column].c_str (), r);
		}
	}

	void drawBackgroundSelected (CDrawContext* context, const CRect& size, CDataBrowser* browser)
	{
		CColor color (browser->getFrame ()->getFocusColor ());
		CView* focusView = browser->getFrame ()->getFocusView ();
		if (!(focusView && browser->isChild (focusView, true)))
		{
			double hue, saturation, value;
			color.toHSV (hue, saturation, value);
			saturation = 0.;
			color.fromHSV (hue, saturation, value);
			color.alpha /= 3;
		}
		context->setDrawMode (kAliasing);
		context->setFillColor (color);
		context->drawRect (size, kDrawFilled);
	}

	void dbDrawCell (CDrawContext* context, const CRect& size, int32_t row, int32_t column, int32_t flags, CDataBrowser* browser)
	{
		if (flags & kRowSelected)
			drawBackgroundSelected (context, size, browser);
		if (row >= dbGetNumRows (browser)-1)
		{
			if (column == 0)
			{
				context->setFontColor (kRedCColor);
				context->setFont (kNormalFont);
				context->drawString ("<< Insert New >>", size);
			}
		}
		else if (column == 0)
		{
			context->setFontColor (kWhiteCColor);
			context->setFont (kNormalFont);
			context->drawString (names[row]->c_str (), size);
		}
		else if (column == dbGetNumColumns (browser)-1)
		{
			if (path == 0)
			{
				path = context->createGraphicsPath ();
				if (path)
				{
					path->addEllipse (CRect (0, 0, 1, 1));
					path->beginSubpath (CPoint (0.3, 0.3));
					path->addLine (CPoint (0.7, 0.7));
					path->beginSubpath (CPoint (0.3, 0.7));
					path->addLine (CPoint (0.7, 0.3));
				}
			}
			CRect r (size);
			r.inset (4, 4);
			CGraphicsTransform t;
			t.translate (r.left, r.top);
			t.scale (r.getWidth (), r.getHeight ());
			context->setFrameColor (mouseRow == row ? kRedCColor : kGreyCColor);
			context->setLineWidth (1.5);
			context->setDrawMode (kAntiAliasing);
			context->drawGraphicsPath (path, CDrawContext::kPathStroked, &t);
		}
		else
		{
			std::string text;
			getCellText (row, column, text, browser);
			context->setFontColor (kWhiteCColor);
			context->setFont (kNormalFont);
			context->drawString (text.c_str (), size);
		}
	}

	virtual bool getCellText (int32_t row, int32_t column, std::string& result, CDataBrowser* browser)	// return true if cell is editable
	{
		if (row >= dbGetNumRows (browser)-1)
		{
			if (column == 0)
			{
				result = "<< Insert New >>";
				return false;
			}
		}
		else if (column == 0)
		{
			result = *names[row];
			return true;
		}
		else if (column == dbGetNumColumns (browser)-1)
		{
			result = "x";
			return false;
		}
		return false;
	}

	void dbCellSetupTextEdit (int32_t row, int32_t column, CTextEdit* textEditControl, CDataBrowser* browser)
	{
		textEditControl->setBackColor (kWhiteCColor);
		textEditControl->setFrameColor (kBlackCColor);
		textEditControl->setFontColor (kBlackCColor);
		textEditControl->setFont (kNormalFont);
		CRect size = textEditControl->getViewSize (size);
		size.inset (1, 1);
		textEditControl->setViewSize (size, true);
		textEditControl->setMouseableArea (size);
	}

	CMouseEventResult dbOnMouseDown (const CPoint& where, const CButtonState& buttons, int32_t row, int32_t column, CDataBrowser* browser)
	{
		if (row == dbGetNumRows (browser)-1)
		{
			if (column == 0)
				browser->beginTextEdit (row, column, "");
		}
		else
		{
			std::string str;
			if (getCellText (row, column, str, browser))
				browser->beginTextEdit (row, column, str.c_str ());
		}
		return kMouseEventHandled;
	}

	CMouseEventResult dbOnMouseMoved (const CPoint& where, const CButtonState& buttons, int32_t row, int32_t column, CDataBrowser* browser)
	{
		if (column == dbGetNumColumns (browser)-1)
		{
			if (mouseRow != row)
			{
				browser->invalidateRow (mouseRow);
				mouseRow = row;
				browser->invalidateRow (mouseRow);
			}
		}
		else if (mouseRow != -1)
		{
			browser->invalidateRow (mouseRow);
			mouseRow = -1;
		}
		return kMouseEventHandled;
	}

	virtual bool startEditing (int32_t row, CDataBrowser* browser)
	{
		if (row == dbGetNumRows (browser)-1)
		{
			browser->beginTextEdit (row, 0, "");
			return true;
		}
		else if (row != -1)
		{
			std::string str;
			if (getCellText (row, 1, str, browser))
			{
				browser->beginTextEdit (row, 1, str.c_str ());
				return true;
			}
		}
		return false;
	}
	
	int32_t dbOnKeyDown (const VstKeyCode& key, CDataBrowser* browser)
	{
		if (key.virt == VKEY_RETURN)
		{
			int32_t row = browser->getSelectedRow ();
			if (startEditing (row, browser))
				return 1;
		}
		
		return -1;
	}


protected:
	UIDescription* desc;
	IActionOperator* actionOperator;
	std::vector<const std::string*> names;
	std::vector<std::string> headerTitles;
	int32_t mouseRow;
	CGraphicsPath* path;
	CDataBrowser* db;
};

//-----------------------------------------------------------------------------
class BitmapBrowserDelegate : public BrowserDelegateBase
//-----------------------------------------------------------------------------
{
public:
	BitmapBrowserDelegate (UIDescription* desc, IActionOperator* actionOperator)
	: BrowserDelegateBase (desc, actionOperator)
	{
		updateNames ();
		headerTitles.push_back ("Name");
		headerTitles.push_back ("Bitmap");
		headerTitles.push_back ("NinePartOffsets");
	}

	CMessageResult notify (CBaseObject* obj, IdStringPtr message)
	{
		if (message == UIDescription::kMessageBitmapChanged)
		{
			updateNames ();
			return kMessageNotified;
		}
		return kMessageUnknown;
	}
	
	void getNames (std::list<const std::string*>& _names)
	{
		desc->collectBitmapNames (_names);
	}
	
	int32_t dbGetNumColumns (CDataBrowser* browser)
	{
		return 4;
	}

	CCoord dbGetCurrentColumnWidth (int32_t index, CDataBrowser* browser)
	{
		if (index > 2)
			return 20;
		return (browser->getWidth () - 40) / 3;
	}
	
	bool getCellText (int32_t row, int32_t column, std::string& result, CDataBrowser* browser)
	{
		if (column == 1)
		{
			CBitmap* bitmap = desc->getBitmap (names[row]->c_str ());
			if (bitmap)
			{
				result = bitmap->getResourceDescription ().u.name;
				return true;
			}
		}
		else if (column == 2)
		{
			CBitmap* bitmap = desc->getBitmap (names[row]->c_str ());
			CNinePartTiledBitmap* nptBitmap = bitmap ? dynamic_cast<CNinePartTiledBitmap*> (bitmap) : 0;
			if (nptBitmap)
			{
				CNinePartTiledBitmap::PartOffsets offsets = nptBitmap->getPartOffsets ();
				std::stringstream stream;
				stream << offsets.left;
				stream << ", ";
				stream << offsets.top;
				stream << ", ";
				stream << offsets.right;
				stream << ", ";
				stream << offsets.bottom;
				result = stream.str ();
				return true;
			}
		}
		return BrowserDelegateBase::getCellText (row, column, result, browser);
	}

	CMouseEventResult dbOnMouseDown (const CPoint& where, const CButtonState& buttons, int32_t row, int32_t column, CDataBrowser* browser)
	{
		if (row < (dbGetNumRows (browser) - 1))
		{
			if (column == dbGetNumColumns (browser) - 1)
			{
				std::string bitmapName (*names[row]);
				actionOperator->performBitmapChange (bitmapName.c_str (), 0, true);
				return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
			}
			if (column == 2)
			{
				std::string str;
				getCellText (row, column, str, browser);
				browser->beginTextEdit (row, column, str.c_str ());
				return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
			}
		}
		return BrowserDelegateBase::dbOnMouseDown (where, buttons, row, column, browser);
	}

	bool parseRect (const std::string& str, CRect& r)
	{
		size_t sep = str.find (',', 0);
		if (sep != std::string::npos)
		{
			r.left = strtol (str.c_str (), 0, 10);
			size_t sep2 = str.find (',', sep);
			if (sep2 != std::string::npos)
			{
				r.top = strtol (str.c_str () + sep2+1, 0, 10);
				sep = str.find (',', sep2+1);
				if (sep != std::string::npos)
				{
					r.right = strtol (str.c_str () + sep+1, 0, 10);
					sep2 = str.find (',', sep+1);
					if (sep2 != std::string::npos)
					{
						r.bottom = strtol (str.c_str () + sep2+1, 0, 10);
						return true;
					}
				}
			}
		}
		return false;
	}

	void dbCellTextChanged (int32_t row, int32_t column, UTF8StringPtr newText, CDataBrowser* browser)
	{
		if (column != 2 && (newText == 0 || strlen (newText) == 0))
			return;
		std::string str;
		if (getCellText (row, column, str, browser) && str == newText)
			return;
		if (column == 0)
		{
			if (row == dbGetNumRows (browser)-1)
			{
				if (!desc->getBitmap (newText))
				{
					actionOperator->performBitmapChange (newText, "not yet defined");
					for (int32_t i = 0; i < (int32_t)names.size (); i++)
					{
						if (*names[i] == newText)
						{
							browser->beginTextEdit ((int32_t)i, 1, "not yet defined");
							break;
						}
					}
					return;
				}
			}
			else
			{
				std::string bitmapName (*names[row]);
				actionOperator->performBitmapNameChange (bitmapName.c_str (), newText);
			}
		}
		else if (column == 1)
		{
			std::string bitmapName (*names[row]);
			actionOperator->performBitmapChange (bitmapName.c_str (), newText);
		}
		else if (column == 2)
		{
			std::string bitmapName (*names[row]);
			str = newText;
			CRect r;
			if (parseRect (str, r))
				actionOperator->performBitmapNinePartTiledChange (bitmapName.c_str (), &r);
			else
				actionOperator->performBitmapNinePartTiledChange (bitmapName.c_str (), 0);
		}
	}
};

//-----------------------------------------------------------------------------
class ColorBrowserDelegate : public BrowserDelegateBase, public IPlatformColorChangeCallback
//-----------------------------------------------------------------------------
{
public:
	ColorBrowserDelegate (UIDescription* desc, IActionOperator* actionOperator) 
	: BrowserDelegateBase (desc, actionOperator), browser (0) 
	{
		updateNames ();
		headerTitles.push_back ("Name");
		headerTitles.push_back ("Color");
	}

	~ColorBrowserDelegate ()
	{
		PlatformUtilities::colorChooser (0, this);
	}
	
	CMessageResult notify (CBaseObject* obj, IdStringPtr message)
	{
		if (message == UIDescription::kMessageColorChanged)
		{
			updateNames ();
			return kMessageNotified;
		}
		return kMessageUnknown;
	}
	
	void getNames (std::list<const std::string*>& _names)
	{
		lastChoosenRow = -1;
		desc->collectColorNames (_names);
	}
	
	bool getCellText (int32_t row, int32_t column, std::string& result, CDataBrowser* browser)
	{
		if (column == 1)
		{
			CColor color;
			if (desc->getColor (names[row]->c_str (), color))
			{
				uint8_t red = color.red;
				uint8_t green = color.green;
				uint8_t blue = color.blue;
				uint8_t alpha = color.alpha;
				char strBuffer[10];
				sprintf (strBuffer, "#%02x%02x%02x%02x", red, green, blue, alpha);
				result = strBuffer;
				return true;
			}
		}
		return BrowserDelegateBase::getCellText (row, column, result, browser);
	}

	bool startEditing (int32_t row, CDataBrowser* browser)
	{
		if (row < (dbGetNumRows (browser) - 1))
		{
			CColor color;
			if (desc->getColor (names[row]->c_str (), color))
			{
				this->browser = browser;
				lastChoosenRow = row;
				PlatformUtilities::colorChooser (&color, this);
			}
			return true;
		}
		return BrowserDelegateBase::startEditing (row, browser);
	}
	
	CMouseEventResult dbOnMouseDown (const CPoint& where, const CButtonState& buttons, int32_t row, int32_t column, CDataBrowser* browser)
	{
		if (row < (dbGetNumRows (browser) - 1))
		{
			if (column == 1)
			{
				CColor color;
				if (desc->getColor (names[row]->c_str (), color))
				{
					this->browser = browser;
					lastChoosenRow = row;
					PlatformUtilities::colorChooser (&color, this);
				}
				return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
			}
			else if (column == 2)
			{
				actionOperator->performColorChange (names[row]->c_str (), MakeCColor (0, 0, 0, 0), true);
				return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
			}
		}
		return BrowserDelegateBase::dbOnMouseDown (where, buttons, row, column, browser);
	}

	void colorChanged (const CColor& color)
	{
		if (lastChoosenRow != -1 && names.size () > (size_t)lastChoosenRow)
		{
			std::string colorName (*names[lastChoosenRow]);
			int32_t temp = lastChoosenRow;
			actionOperator->performColorChange (colorName.c_str (), color);
			lastChoosenRow = temp;
			browser->invalidateRow (lastChoosenRow);
		}
	}
	
	void dbCellTextChanged (int32_t row, int32_t column, UTF8StringPtr newText, CDataBrowser* browser)
	{
		if (newText == 0 || strlen (newText) == 0)
			return;
		std::string str;
		if (getCellText (row, column, str, browser) && str == newText)
			return;
		if (column == 0)
		{
			if (row == dbGetNumRows (browser)-1)
			{
				CColor color;
				if (!desc->getColor (newText, color))
				{
					color = MakeCColor (1, 2, 3, 4);
					actionOperator->performColorChange (newText, color);
				}
			}
			else
			{
				std::string colorName (*names[row]);
				actionOperator->performColorNameChange (colorName.c_str (), newText);
			}
		}
		else if (column == 1)
		{
			std::string colorString (newText);
			CColor newColor;
			if (UIDescription::parseColor (colorString, newColor))
			{
				std::string colorName (*names[row]);
				actionOperator->performColorChange (colorName.c_str (), newColor);
			}
		}
	}

	void dbDrawCell (CDrawContext* context, const CRect& size, int32_t row, int32_t column, int32_t flags, CDataBrowser* browser)
	{
		if (column == 1 && row < (dbGetNumRows (browser) - 1))
		{
			if (flags & kRowSelected)
				drawBackgroundSelected (context, size, browser);
			CColor color;
			if (desc->getColor (names[row]->c_str (), color))
			{
				CRect colorRect (size);
				colorRect.setWidth (colorRect.getHeight ());
				colorRect.inset (2, 2);
				context->setFillColor (color);
				context->drawRect (colorRect, kDrawFilled);

				uint8_t red = color.red;
				uint8_t green = color.green;
				uint8_t blue = color.blue;
				uint8_t alpha = color.alpha;
				char strBuffer[10];
				sprintf (strBuffer, "#%02x%02x%02x%02x", red, green, blue, alpha);
				context->setFontColor (kWhiteCColor);
				context->setFont (kNormalFont);
				context->drawString (strBuffer, size);
			}
			return;
		}
		BrowserDelegateBase::dbDrawCell (context, size, row, column, flags, browser);
	}
	int32_t lastChoosenRow;
	CDataBrowser* browser;
};

//-----------------------------------------------------------------------------
class TagBrowserDelegate : public BrowserDelegateBase
//-----------------------------------------------------------------------------
{
public:
	TagBrowserDelegate (UIDescription* desc, IActionOperator* actionOperator)
	: BrowserDelegateBase (desc, actionOperator)
	{
		updateNames ();
		headerTitles.push_back ("Name");
		headerTitles.push_back ("Tag");
	}

	CMessageResult notify (CBaseObject* obj, IdStringPtr message)
	{
		if (message == UIDescription::kMessageTagChanged)
		{
			updateNames ();
			return kMessageNotified;
		}
		return kMessageUnknown;
	}
	
	void getNames (std::list<const std::string*>& _names)
	{
		desc->collectControlTagNames (_names);
	}
	
	bool getCellText (int32_t row, int32_t column, std::string& result, CDataBrowser* browser)
	{
		if (column == 1 && row < (dbGetNumRows (browser) - 1))
		{
			std::stringstream str;
			str << desc->getTagForName (names[row]->c_str ());
			result = str.str ();
			return true;
		}
		return BrowserDelegateBase::getCellText (row, column, result, browser);
	}

	CMouseEventResult dbOnMouseDown (const CPoint& where, const CButtonState& buttons, int32_t row, int32_t column, CDataBrowser* browser)
	{
		if (row < (dbGetNumRows (browser) - 1))
		{
			if (column == 2)
			{
				std::string tagName (*names[row]);
				actionOperator->performTagChange (tagName.c_str (), 0, true);
				return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
			}
		}
		return BrowserDelegateBase::dbOnMouseDown (where, buttons, row, column, browser);
	}

	void dbCellTextChanged (int32_t row, int32_t column, UTF8StringPtr newText, CDataBrowser* browser)
	{
		if (newText == 0 || strlen (newText) == 0)
			return;
		std::string str;
		if (getCellText (row, column, str, browser) && str == newText)
			return;
		if (column == 0)
		{
			if (row == dbGetNumRows (browser)-1)
			{
				if (desc->getTagForName (newText) == -1)
				{
					actionOperator->performTagChange (newText, -2);
					for (int32_t i = 0; i < (int32_t)names.size (); i++)
					{
						if (*names[i] == newText)
						{
							browser->makeRowVisible (i);
							browser->beginTextEdit (i, 1, "-2");
							break;
						}
					}
					return;
				}
			}
			else
			{
				std::string tagName (*names[row]);
				actionOperator->performTagNameChange (tagName.c_str (), newText);
			}
		}
		else if (column == 1)
		{
			int32_t tag = (int32_t)strtol (newText, 0, 10);
			actionOperator->performTagChange (names[row]->c_str (), tag);
		}
	}
};


//-----------------------------------------------------------------------------
class FontBrowserDelegate : public BrowserDelegateBase, public IFontChooserDelegate
//-----------------------------------------------------------------------------
{
public:
	FontBrowserDelegate (UIDescription* desc, IActionOperator* actionOperator)
	: BrowserDelegateBase (desc, actionOperator), browser (0)
	{
		updateNames ();
		headerTitles.push_back ("Name");
		headerTitles.push_back ("Font");
	}

	~FontBrowserDelegate ()
	{
		UIFontChooserPanel::hide ();
	}
	
	CMessageResult notify (CBaseObject* obj, IdStringPtr message)
	{
		if (message == UIDescription::kMessageFontChanged)
		{
			updateNames ();
			return kMessageNotified;
		}
		return kMessageUnknown;
	}
	
	void getNames (std::list<const std::string*>& _names)
	{
		desc->collectFontNames (_names);
	}
	
	bool getCellText (int32_t row, int32_t column, std::string& result, CDataBrowser* browser)
	{
		if (row < (dbGetNumRows (browser) - 1))
		{
			CFontRef font = desc->getFont (names[row]->c_str ());
			if (!font)
				return false;
			if (column == 1)
			{
				std::stringstream str;
				str << font->getName ();
				int32_t fstyle = font->getStyle ();
				if (fstyle != 0)
					str << " [";
				if (fstyle & kBoldFace)
					str << "b";
				if (fstyle & kItalicFace)
					str << "i";
				if (fstyle & kUnderlineFace)
					str << "u";
				if (fstyle & kStrikethroughFace)
					str << "s";
				if (fstyle != 0)
					str << "]";
				str << " ";
				str << font->getSize ();
				result = str.str ();
				return true;
			}
		}
		return BrowserDelegateBase::getCellText (row, column, result, browser);
	}

	void fontChanged (CFontChooser* chooser, CFontRef newFont)
	{
		std::string fontName (*names[lastChoosenRow]);
		actionOperator->performFontChange (fontName.c_str (), newFont);
	}
	
	bool startEditing (int32_t row, CDataBrowser* browser)
	{
		if (row < (dbGetNumRows (browser) - 1))
		{
			std::string fontName (*names[row]);
			CFontRef currentFont = desc->getFont (fontName.c_str ());
			this->browser = browser;
			lastChoosenRow = row;
			UIFontChooserPanel::show (currentFont, this, browser->getFrame ()->getPlatformFrame ()->getPlatformRepresentation ());
			return true;
		}
		return BrowserDelegateBase::startEditing (row, browser);
	}
	
	CMouseEventResult dbOnMouseDown (const CPoint& where, const CButtonState& buttons, int32_t row, int32_t column, CDataBrowser* browser)
	{
		if (row < (dbGetNumRows (browser) - 1))
		{
			if (column == dbGetNumColumns (browser) - 1)
			{
				actionOperator->performFontChange (names[row]->c_str (), 0, true);
				return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
			}
			else if (column == 1)
			{
				std::string fontName (*names[row]);
				CFontRef currentFont = desc->getFont (fontName.c_str ());
				this->browser = browser;
				lastChoosenRow = row;
				UIFontChooserPanel::show (currentFont, this, browser->getFrame ()->getPlatformFrame ()->getPlatformRepresentation ());
				return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
			}
		}
		return BrowserDelegateBase::dbOnMouseDown (where, buttons, row, column, browser);
	}

	void dbCellTextChanged (int32_t row, int32_t column, UTF8StringPtr newText, CDataBrowser* browser)
	{
		if (newText == 0 || strlen (newText) == 0)
			return;
		std::string str;
		if (getCellText (row, column, str, browser) && str == newText)
			return;
		if (column == 0)
		{
			if (row == dbGetNumRows (browser)-1)
			{
				if (!desc->getFont (newText))
				{
					CFontRef font = new CFontDesc (*kSystemFont);
					actionOperator->performFontChange (newText, font);
					font->forget ();
				}
			}
			else
			{
				std::string fontName (*names[row]);
				actionOperator->performFontNameChange (fontName.c_str (), newText);
			}
		}
	}
	int32_t lastChoosenRow;
	CDataBrowser* browser;
};

static const CViewAttributeID attrNameID = 'atnm';

//-----------------------------------------------------------------------------
static void setViewAttributeName (CView* view, UTF8StringPtr name)
{
	view->setAttribute (attrNameID, (int32_t)strlen (name) + 1, name);
}

//-----------------------------------------------------------------------------
static bool getViewAttributeName (CView* view, std::string& name)
{
	bool result = false;
	int32_t attrSize = 0;
	if (view->getAttributeSize (attrNameID, attrSize))
	{
		char* attrNameCStr = new char [attrSize];
		if (view->getAttribute (attrNameID, attrSize, attrNameCStr, attrSize))
		{
			name = attrNameCStr;
			result = true;
		}
		delete [] attrNameCStr;
	}
	return result;
}

//-----------------------------------------------------------------------------
static void updateMenuFromList (COptionMenu* menu, std::list<const std::string*>& names, const std::string& defaultValue, bool addNoneItem = false)
{
	menu->removeAllEntry ();
	names.sort (std__stringCompare);
	int32_t current = -1;
	std::list<const std::string*>::const_iterator it = names.begin ();
	while (it != names.end ())
	{
		menu->addEntry (new CMenuItem ((*it)->c_str ()));
		if (*(*it) == defaultValue)
			current = menu->getNbEntries () - 1;
		it++;
	}
	menu->setValue ((float)current);
	if (addNoneItem)
	{
		menu->addSeparator ();
		menu->addEntry (new CMenuItem ("None", -1000));
		if (current == -1)
			menu->setValue (menu->getNbEntries () - 1.f);
	}
	else if (current == -1)
	{
		menu->addEntry (new CMenuItem (defaultValue.c_str ()));
		menu->setValue (menu->getNbEntries () - 1.f);
	}
}

//-----------------------------------------------------------------------------
COptionMenu* UIViewInspector::createMenuFromList (const CRect& size, CControlListener* listener, std::list<const std::string*>& names, const std::string& defaultValue, bool addNoneItem)
{
	COptionMenu* menu = new FocusOptionMenu (size, listener, -1);
	menu->setStyle (kCheckStyle|kPopupStyle);
	updateMenuFromList (menu, names, defaultValue, addNoneItem);
	return menu;
}

//-----------------------------------------------------------------------------
UIViewInspector::UIViewInspector (UISelection* selection, IActionOperator* actionOperator, void* parentPlatformWindow)
: selection (selection)
, actionOperator (actionOperator)
, description (0)
, attributesView (0)
, viewNameLabel (0)
, scrollView (0)
, platformWindow (0)
, parentPlatformWindow (parentPlatformWindow)
{
	selection->remember ();
	selection->addDependency (this);
}

//-----------------------------------------------------------------------------
UIViewInspector::~UIViewInspector ()
{
	hide ();
	setUIDescription (0);
	selection->removeDependency (this);
	selection->forget ();
}

//-----------------------------------------------------------------------------
void UIViewInspector::setUIDescription (UIDescription* desc)
{
	if (description != desc)
	{
		if (description)
		{
			description->forget ();
		}
		description = desc;
		if (description)
		{
			UIAttributes* attr = description->getCustomAttributes ("UIViewInspector");
			if (attr)
				attr->getRectAttribute ("windowSize", windowSize);
			description->remember ();
		}
	}
}

//-----------------------------------------------------------------------------
void UIViewInspector::updateAttributeValueView (const std::string& attrName)
{
}

//-----------------------------------------------------------------------------
void UIViewInspector::addColorBitmapsToColorMenu (COptionMenu* menu, IUIDescription* desc)
{
	CMenuItemList* items = menu->getItems ();
	CMenuItemList::iterator it = items->begin ();
	CColor color;
	const CCoord size = 15;
	while (it != items->end ())
	{
		CMenuItem* item = (*it);
		
		if (desc->getColor (item->getTitle (), color))
		{
			COffscreenContext* context = COffscreenContext::create (getFrame (), size, size);
			if (context)
			{
				context->beginDraw ();
				context->setFillColor (color);
				context->drawRect (CRect (0, 0, size, size), kDrawFilled);
				context->endDraw ();
				item->setIcon (context->getBitmap ());
				context->forget ();
			}
		}
		it++;
	}
}

//-----------------------------------------------------------------------------
CView* UIViewInspector::createViewForAttribute (const std::string& attrName, CCoord width)
{
	if (description == 0)
		return 0;
	UIViewFactory* viewFactory = dynamic_cast<UIViewFactory*> (description->getViewFactory ());
	if (viewFactory == 0)
		return 0;

	const CCoord height = 20;

	CViewContainer* container = new CViewContainer (CRect (0, 0, width, height+2), 0);
	container->setAutosizeFlags (kAutosizeLeft|kAutosizeRight|kAutosizeColumn);
	container->setTransparency (true);

	CCoord middle = width/2;

	CTextLabel* label = new CTextLabel (CRect (5, 1, middle - 10, height+1), attrName.c_str ());
	label->setTransparency (true);
	label->setHoriAlign (kRightText);
	label->setFontColor (kWhiteCColor);
	label->setFont (kNormalFont);
	container->addView (label);

	bool hasDifferentValues = false;
	CRect r (middle+10, 1, width-5, height+1);
	CView* valueView = 0;
	IViewCreator::AttrType attrType = viewFactory->getAttributeType (*selection->begin (), attrName);
	std::string attrValue;
	bool first = true;
	FOREACH_IN_SELECTION(selection, view)
		std::string temp;
		viewFactory->getAttributeValue (view, attrName, temp, description);
		if (temp != attrValue && !first)
			hasDifferentValues = true;
		attrValue = temp;
		first = false;
	FOREACH_IN_SELECTION_END
	switch (attrType)
	{
		case IViewCreator::kColorType:
		{
			std::list<const std::string*> names;
			description->collectColorNames (names);
			COptionMenu* menu = createMenuFromList (r, this, names, attrValue);
			if (menu->getValue () == -1)
			{
				menu->addEntry (new CMenuItem (attrValue.c_str ()));
				menu->setValue ((float)menu->getNbEntries ());
			}
			addColorBitmapsToColorMenu (menu, description);
			valueView = menu;
			break;
		}
		case IViewCreator::kFontType:
		{
			std::list<const std::string*> names;
			description->collectFontNames (names);
			COptionMenu* menu = createMenuFromList (r, this, names, attrValue);
			valueView = menu;
			break;
		}
		case IViewCreator::kBitmapType:
		{
			std::list<const std::string*> names;
			description->collectBitmapNames (names);
			COptionMenu* menu = createMenuFromList (r, this, names, attrValue, true);
			valueView = menu;
			break;
		}
		case IViewCreator::kTagType:
		{
			std::list<const std::string*> names;
			description->collectControlTagNames (names);
			COptionMenu* menu = createMenuFromList (r, this, names, attrValue, true);
			valueView = menu;
			break;
		}
		case IViewCreator::kBooleanType:
		{
			SimpleBooleanButton* booleanButton = new SimpleBooleanButton (r, this);
			if (attrValue == "true")
				booleanButton->setValue (1);
			valueView = booleanButton;
			break;
		}
		default:
		{
			CTextEdit* textEdit = new CTextEdit (r, this, -1);
			textEdit->setText (attrValue.c_str ());
			valueView = textEdit;
		}
	}
	if (valueView)
	{
		CParamDisplay* paramDisplay = dynamic_cast<CParamDisplay*> (valueView);
		if (paramDisplay)
		{
			paramDisplay->setHoriAlign (kLeftText);
			paramDisplay->setBackColor (hasDifferentValues ? uidViewAttributeDifferentValuesBackgroundColor : uidViewAttributeValueBackgroundColor);
			paramDisplay->setFrameColor (uidViewAttributeValueFrameColor);
			paramDisplay->setFontColor (kBlackCColor);
			paramDisplay->setFont (kNormalFont);
			paramDisplay->setStyle (paramDisplay->getStyle ());
			paramDisplay->setTextInset (CPoint (4,0));
		}
		setViewAttributeName (valueView, attrName.c_str ());
		container->addView (valueView);
		attributeViews.push_back (valueView);
	}
	return container;
}

//-----------------------------------------------------------------------------
void UIViewInspector::updateAttributeViews ()
{
	if (description == 0)
		return;
	UIViewFactory* viewFactory = dynamic_cast<UIViewFactory*> (description->getViewFactory ());
	if (viewFactory == 0)
		return;

	std::string attrName;
	std::string attrValue;
	std::list<CView*>::const_iterator it = attributeViews.begin ();
	while (it != attributeViews.end ())
	{
		CView* view = (*it);
		if (view && getViewAttributeName (view, attrName))
		{
			attrValue = "";
			viewFactory->getAttributeValue (*selection->begin (), attrName, attrValue, description);
			CTextEdit* textEdit = dynamic_cast<CTextEdit*> (view);
			COptionMenu* optMenu = dynamic_cast<COptionMenu*> (view);
			SimpleBooleanButton* booleanButton = dynamic_cast<SimpleBooleanButton*> (view);
			if (textEdit)
			{
				textEdit->setText (attrValue.c_str ());
				if (textEdit->isDirty ())
					textEdit->invalid ();
			}
			else if (optMenu)
			{
				IViewCreator::AttrType type = viewFactory->getAttributeType (*selection->begin (), attrName);
				bool addNoneItem = false;
				std::list<const std::string*> names;
				switch (type)
				{
					case IViewCreator::kColorType:
					{
						description->collectColorNames (names);
						break;
					}
					case IViewCreator::kFontType:
					{
						description->collectFontNames (names);
						break;
					}
					case IViewCreator::kBitmapType:
					{
						description->collectBitmapNames (names);
						addNoneItem = true;
						break;
					}
					case IViewCreator::kTagType:
					{
						description->collectControlTagNames (names);
						addNoneItem = true;
						break;
					}
					default:
						break;
				}
				updateMenuFromList (optMenu, names, attrValue, addNoneItem);
				if (type == IViewCreator::kColorType)
					addColorBitmapsToColorMenu (optMenu, description);
			}
			else if (booleanButton)
			{
				float newValue = attrValue == "true" ? 1.f : 0.f;
				if (newValue != booleanButton->getValue ())
				{
					booleanButton->setValue (newValue);
					booleanButton->invalid ();
				}
			}
		}
		it++;
	}
}

//-----------------------------------------------------------------------------
CView* UIViewInspector::createAttributesView (CCoord width)
{
	if (description == 0)
		return 0;
	UIViewFactory* viewFactory = dynamic_cast<UIViewFactory*> (description->getViewFactory ());
	if (viewFactory == 0)
		return 0;
	CRect size (0, 0, width, 400);
	if (attributesView == 0)
	{
		attributesView = new CViewContainer (size, 0);
		attributesView->setTransparency (true);
		attributesView->setAutosizeFlags (kAutosizeAll);
		viewNameLabel = new CTextLabel (CRect (0, 3, width, 18));
		viewNameLabel->setStyle (kNoFrame);
		viewNameLabel->setTextInset (CPoint (5, 0));
		viewNameLabel->setHoriAlign (kLeftText);
		CFontDesc* labelFont = new CFontDesc (*kSystemFont);
		labelFont->setSize (10);
		viewNameLabel->setFont (labelFont);
		labelFont->forget ();
		viewNameLabel->setAutosizeFlags (kAutosizeLeft|kAutosizeRight);
		viewNameLabel->setBackColor (uidDataBrowserLineColor);
		attributesView->addView (viewNameLabel);
	}
	if (scrollView == 0)
	{
		size.top += 20;
		scrollView = new CScrollView (size, CRect (0, 0, 0, 0), 0, CScrollView::kVerticalScrollbar|CScrollView::kDontDrawFrame, 10);
		scrollView->setBackgroundColor (kTransparentCColor);
		scrollView->setAutosizeFlags (kAutosizeAll);
		CScrollbar* bar = scrollView->getVerticalScrollbar ();
		bar->setScrollerColor (uidScrollerColor);
		bar->setBackgroundColor (kTransparentCColor);
		bar->setFrameColor (kTransparentCColor);
		attributesView->addView (scrollView);
	}
	else
	{
		scrollView->setContainerSize (CRect (0, 0, 0, 0));
		width = scrollView->getWidth ();
		size.setWidth (width);
		attributeViews.clear ();
		scrollView->removeAll ();
	}
	CCoord viewLocation = 0;
	CCoord containerWidth = width - 10;
	int32_t selectedViews = selection->total ();
	if (selectedViews > 0)
	{
		if (selectedViews == 1)
		{
			CView* view = selection->first ();
			UTF8StringPtr viewname = viewFactory->getViewName (view);
			if (viewname == 0)
				viewname = typeid(*view).name ();
			viewNameLabel->setText (viewname);
		}
		else
		{
			viewNameLabel->setText ("Multiple Selection");
		}
		std::list<std::string> attrNames;
		FOREACH_IN_SELECTION(selection, view)
			std::list<std::string> temp;
			if (viewFactory->getAttributeNamesForView (view, temp))
			{
				if (attrNames.size () == 0)
					attrNames = temp;
				else
				{
					std::list<std::string> toRemove;
					std::list<std::string>::const_iterator it = attrNames.begin ();
					while (it != attrNames.end ())
					{
						bool found = false;
						std::list<std::string>::const_iterator it2 = temp.begin ();
						while (it2 != temp.end ())
						{
							if ((*it) == (*it2))
							{
								found = true;
								break;
							}
							it2++;
						}
						if (!found)
							toRemove.push_back (*it);
						it++;
					}
					it = toRemove.begin ();
					while (it != toRemove.end ())
					{
						attrNames.remove (*it);
						it++;
					}
				}
			}
		FOREACH_IN_SELECTION_END
		std::list<std::string>::const_iterator it = attrNames.begin ();
		while (it != attrNames.end ())
		{
			CView* view = createViewForAttribute ((*it), containerWidth);
			if (view)
			{
				CRect viewSize = view->getViewSize ();
				viewSize.offset (0, viewLocation);
				view->setViewSize (viewSize);
				view->setMouseableArea (viewSize);
				viewLocation += view->getHeight () + 2;
				scrollView->addView (view);
			}
			it++;
		}
	}
	else
	{
		viewNameLabel->setText ("No Selection");
	}

	size.setHeight (viewLocation);
	scrollView->setContainerSize (size);
	attributesView->invalid ();
	return attributesView;
}

//-----------------------------------------------------------------------------
void UIViewInspector::show ()
{
	if (platformWindow == 0)
	{
		createAttributesView (400);
		if (attributesView == 0)
			return;
		CRect size = attributesView->getViewSize ();
		if (size.getHeight () < 400)
			size.setHeight (400);

		ColorBrowserDelegate* colorDelegate = new ColorBrowserDelegate (description, actionOperator);
		CDataBrowser* colorBrowser = new CDataBrowser (size, 0, colorDelegate, CScrollView::kVerticalScrollbar|CScrollView::kDontDrawFrame|CDataBrowser::kDrawRowLines|CDataBrowser::kDrawColumnLines|CDataBrowser::kDrawHeader, 10);
		colorBrowser->setBackgroundColor (kTransparentCColor);
		colorBrowser->setAutosizeFlags (kAutosizeAll);
		colorDelegate->forget ();
		CScrollbar* bar = colorBrowser->getVerticalScrollbar ();
		bar->setScrollerColor (uidScrollerColor);
		bar->setBackgroundColor (kTransparentCColor);
		bar->setFrameColor (kTransparentCColor);

		BitmapBrowserDelegate* bmpDelegate = new BitmapBrowserDelegate (description, actionOperator);
		CDataBrowser* bitmapBrowser = new CDataBrowser (size, 0, bmpDelegate, CScrollView::kVerticalScrollbar|CScrollView::kDontDrawFrame|CDataBrowser::kDrawRowLines|CDataBrowser::kDrawColumnLines|CDataBrowser::kDrawHeader, 10);
		bitmapBrowser->setBackgroundColor (kTransparentCColor);
		bitmapBrowser->setAutosizeFlags (kAutosizeAll);
		bmpDelegate->forget ();
		bar = bitmapBrowser->getVerticalScrollbar ();
		bar->setScrollerColor (uidScrollerColor);
		bar->setBackgroundColor (kTransparentCColor);
		bar->setFrameColor (kTransparentCColor);

		FontBrowserDelegate* fontDelegate = new FontBrowserDelegate (description, actionOperator);
		CDataBrowser* fontBrowser = new CDataBrowser (size, 0, fontDelegate, CScrollView::kVerticalScrollbar|CScrollView::kDontDrawFrame|CDataBrowser::kDrawRowLines|CDataBrowser::kDrawColumnLines|CDataBrowser::kDrawHeader, 10);
		fontBrowser->setBackgroundColor (kTransparentCColor);
		fontBrowser->setAutosizeFlags (kAutosizeAll);
		fontDelegate->forget ();
		bar = fontBrowser->getVerticalScrollbar ();
		bar->setScrollerColor (uidScrollerColor);
		bar->setBackgroundColor (kTransparentCColor);
		bar->setFrameColor (kTransparentCColor);

		TagBrowserDelegate* tagDelegate = new TagBrowserDelegate (description, actionOperator);
		CDataBrowser* controlTagBrowser = new CDataBrowser (size, 0, tagDelegate, CScrollView::kVerticalScrollbar|CScrollView::kDontDrawFrame|CDataBrowser::kDrawRowLines|CDataBrowser::kDrawColumnLines|CDataBrowser::kDrawHeader, 10);
		controlTagBrowser->setBackgroundColor (kTransparentCColor);
		controlTagBrowser->setAutosizeFlags (kAutosizeAll);
		tagDelegate->forget ();
		bar = controlTagBrowser->getVerticalScrollbar ();
		bar->setScrollerColor (uidScrollerColor);
		bar->setBackgroundColor (kTransparentCColor);
		bar->setFrameColor (kTransparentCColor);

		const CCoord kMargin = 12;
		size.bottom += 50;
		size.offset (kMargin, 0);
		CRect tabButtonSize (0, 0, 400/5, 50);
		CTabView* tabView = new CTabView (size, 0, tabButtonSize, 0, CTabView::kPositionTop);
		tabView->setTabViewInsets (CPoint (5, 1));
		tabView->addTab (attributesView, new InspectorTabButton (tabButtonSize, "Attributes", -1));
		tabView->addTab (bitmapBrowser, new InspectorTabButton (tabButtonSize, "Bitmaps"));
		tabView->addTab (colorBrowser, new InspectorTabButton (tabButtonSize, "Colors"));
		tabView->addTab (fontBrowser, new InspectorTabButton (tabButtonSize, "Fonts"));
		tabView->addTab (controlTagBrowser, new InspectorTabButton (tabButtonSize, "Tags", 1));
		tabView->alignTabs ();
		tabView->setAutosizeFlags (kAutosizeAll);
		tabView->setBackgroundColor (kTransparentCColor);

		size.offset (-kMargin, 0);
		size.right += kMargin*2;
		size.bottom += kMargin;
		platformWindow = PlatformWindow::create (size, "VSTGUI Inspector", PlatformWindow::kPanelType, PlatformWindow::kResizable, this, parentPlatformWindow);
		if (platformWindow)
		{
			#if MAC_CARBON && MAC_COCOA
			CFrame::setCocoaMode (true);
			#endif

			frame = new CFrame (size, platformWindow->getPlatformHandle (), this);
			frame->setBackgroundColor (uidPanelBackgroundColor);

			frame->addView (tabView);
			frame->setFocusDrawingEnabled (true);
			frame->setFocusColor (uidFocusColor);
			frame->setFocusWidth (1.2);
			platformWindow->center ();
			if (windowSize.getWidth () > 0)
				platformWindow->setSize (windowSize);
			platformWindow->show ();
		}
		else
		{
			attributeViews.clear ();
			tabView->forget ();
			scrollView = 0;
			attributesView = 0;
			viewNameLabel = 0;
		}
	}
}

//-----------------------------------------------------------------------------
void UIViewInspector::beforeSave ()
{
	if (platformWindow)
		windowSize = platformWindow->getSize ();
	if (description)
	{
		UIAttributes* attr = description->getCustomAttributes ("UIViewInspector");
		if (!attr)
			attr = new UIAttributes;
		attr->setRectAttribute ("windowSize", windowSize);
		description->setCustomAttributes ("UIViewInspector", attr);
	}
}

//-----------------------------------------------------------------------------
void UIViewInspector::hide ()
{
	if (platformWindow)
	{
		beforeSave ();
		attributeViews.clear ();
		frame->close ();
		frame = 0;
		scrollView = 0;
		attributesView = 0;
		viewNameLabel = 0;
		platformWindow->forget ();
		platformWindow = 0;
	}
}

//-----------------------------------------------------------------------------
void UIViewInspector::valueChanged (CControl* pControl)
{
	if (description == 0)
		return;
	UIViewFactory* viewFactory = dynamic_cast<UIViewFactory*> (description->getViewFactory ());
	if (viewFactory == 0)
		return;

	std::string attrName;
	if (!getViewAttributeName (pControl, attrName))
		return;
	if (attrName.size () > 0)
	{
		std::string attrValue;
		CTextEdit* textEdit = dynamic_cast<CTextEdit*> (pControl);
		COptionMenu* optMenu = dynamic_cast<COptionMenu*> (pControl);
		SimpleBooleanButton* booleanButton = dynamic_cast<SimpleBooleanButton*> (pControl);
		if (textEdit)
		{
			UTF8StringPtr textValueCStr = textEdit->getText ();
			if (textValueCStr)
			{
				attrValue = textValueCStr;
			}
		}
		else if (optMenu)
		{
			int32_t index = optMenu->getLastResult ();
			CMenuItem* item = optMenu->getEntry (index);
			if (item)
			{
				if (item->getTag () == -1000)
					attrValue = "";
				else
					attrValue = item->getTitle ();
			}
			optMenu->setValue ((float)index);
			optMenu->invalid ();
		}
		else if (booleanButton)
		{
			attrValue = booleanButton->getValue () == 0 ? "false" : "true";
		}
		actionOperator->performAction (new AttributeChangeAction (description, selection, attrName, attrValue));
	}
}

//-----------------------------------------------------------------------------
CMessageResult UIViewInspector::notify (CBaseObject* sender, IdStringPtr message)
{
	if (message == UISelection::kMsgSelectionChanged)
	{
		if (frame)
		{
			createAttributesView (400);
		}
		return kMessageNotified;
	}
	else if (message == UISelection::kMsgSelectionViewChanged)
	{
		if (frame)
		{
			updateAttributeViews ();
		}
		return kMessageNotified;
	}
	return kMessageUnknown;
}

//-----------------------------------------------------------------------------
void UIViewInspector::checkWindowSizeConstraints (CPoint& size, PlatformWindow* platformWindow)
{
	if (size.x < 400)
		size.x = 400;
	if (size.y < 200)
		size.y = 200;
}

//-----------------------------------------------------------------------------
void UIViewInspector::windowSizeChanged (const CRect& newSize, PlatformWindow* platformWindow)
{
	frame->setSize (newSize.getWidth (), newSize.getHeight ());
}

//-----------------------------------------------------------------------------
void UIViewInspector::windowClosed (PlatformWindow* platformWindow)
{
}

} // namespace

#endif // VSTGUI_LIVE_EDITING
