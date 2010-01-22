//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins : 
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

#ifndef __cfileselector__
#define __cfileselector__

#include "cframe.h"
#include <list>
#include <vector>

namespace VSTGUI {

//-----------------------------------------------------------------------------
// CFileExtension Declaration
//! @brief file extension description
//-----------------------------------------------------------------------------
class CFileExtension : public CBaseObject
{
public:
	CFileExtension (const char* description, const char* extension, const char* mimeType = 0, int macType = 0);
	CFileExtension (const CFileExtension& ext);
	~CFileExtension ();

	const char* getDescription () const { return description; }
	const char* getExtension () const { return extension; }
	const char* getMimeType () const { return mimeType; }
	int getMacType () const { return macType; }

	bool operator== (const CFileExtension& ext) const;
//-----------------------------------------------------------------------------
	CLASS_METHODS(CFileExtension, CBaseObject)
protected:
	void init (const char* description, const char* extension, const char* mimeType);
	
	char* description;
	char* extension;
	char* mimeType;
	int macType;
};

//-----------------------------------------------------------------------------
// CNewFileSelector Declaration
//! @brief New file selector class
/*! @class CNewFileSelector
@section usage Usage
Running the file selector
@code
void MyClass::runFileSelector ()
{
	CNewFileSelector* selector = CNewFileSelector::create (getFrame (), CNewFileSelector::kSelectFile);
	if (selector)
	{
		selector->addFileExtension (CFileExtension ("AIFF", "aif", "audio/aiff"));
		selector->setDefaultExtension (CFileExtension ("WAVE", "wav"));
		selector->setTitle("Choose An Audio File");
		selector->run (this);
		selector->forget ();
	}
}
@endcode
Getting results
@code
CMessageResult MyClass::notify (CBaseObject* sender, const char* message)
{
	if (message == CNewFileSelector::kSelectEndMessage)
	{
		CNewFileSelector* sel = dynamic_cast<CNewFileSelector*>(sender);
		if (sel)
		{
			// do anything with the selected files here
			return kMessageNotified;
		}
	}
	return parent::notify (sender, message);
}
@endcode
*/
//-----------------------------------------------------------------------------
class CNewFileSelector : public CBaseObject
{
public:
	enum Style {
		kSelectFile,				///< select file(s) selector style
		kSelectSaveFile,			///< select save file selector style
		kSelectDirectory			///< select directory style
	};
	
	//-----------------------------------------------------------------------------
	/// @name CFileSelector running
	//-----------------------------------------------------------------------------
	//@{
	static CNewFileSelector* create (CFrame* parent = 0, Style style = kSelectFile); ///< create a new instance

	bool run (CBaseObject* delegate);	///< the delegate will get a kSelectEndMessage throu the notify method where the sender is this CNewFileSelector object
	void cancel ();						///< cancel running the file selector
	bool runModal ();					///< run as modal dialog
	//@}

	//-----------------------------------------------------------------------------
	/// @name CFileSelector setup
	//-----------------------------------------------------------------------------
	//@{
	void setTitle (const char* title);							///< set title of file selector
	void setInitialDirectory (const char* path);				///< set initial directory (UTF8 string)
	void setDefaultSaveName (const char* name);					///< set initial save name (UTF8 string)
	void setDefaultExtension (const CFileExtension& extension);	///< set default file extension
	void setAllowMultiFileSelection (bool state);				///< set allow multi file selection (only valid for kSelectFile selector style)
	void addFileExtension (const CFileExtension& extension);	///< add a file extension
	//@}

	//-----------------------------------------------------------------------------
	/// @name CFileSelector result
	//-----------------------------------------------------------------------------
	//@{
	int getNumSelectedFiles () const;							///< get number of selected files
	const char* getSelectedFile (unsigned int index) const;		///< get selected file. Result is only valid as long as the instance of CNewFileSelector is valid.
	//@}

	static const CFileExtension& getAllFilesExtension ();		///< get the all files extension

	static const char* kSelectEndMessage;
//-----------------------------------------------------------------------------
	CLASS_METHODS_NOCOPY(CNewFileSelector, CBaseObject)
protected:
	CNewFileSelector (CFrame* frame = 0);
	~CNewFileSelector ();

	virtual bool runInternal (CBaseObject* delegate) = 0;
	virtual void cancelInternal () = 0;
	virtual bool runModalInternal () = 0;

	CFrame* frame;
	char* title;
	char* initialPath;
	char* defaultSaveName;
	const CFileExtension* defaultExtension;
	bool allowMultiFileSelection;

	std::list<CFileExtension> extensions;
	std::vector<char*> result;
};

} // namespace

#endif
