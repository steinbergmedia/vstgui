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

#ifndef __cfileselector__
#define __cfileselector__

#include "vstguifwd.h"
#include <list>
#include <vector>
#if VSTGUI_HAS_FUNCTIONAL
#include <functional>
#endif

namespace VSTGUI {

//-----------------------------------------------------------------------------
// CFileExtension Declaration
//! @brief file extension description
//-----------------------------------------------------------------------------
class CFileExtension : public CBaseObject
{
public:
	CFileExtension (UTF8StringPtr description, UTF8StringPtr extension, UTF8StringPtr mimeType = 0, int32_t macType = 0, UTF8StringPtr uti = 0);
	CFileExtension (const CFileExtension& ext);
	~CFileExtension ();

	UTF8StringPtr getDescription () const { return description; }
	UTF8StringPtr getExtension () const { return extension; }
	UTF8StringPtr getMimeType () const { return mimeType; }
	UTF8StringPtr getUTI () const { return uti; }
	int32_t getMacType () const { return macType; }

	bool operator== (const CFileExtension& ext) const;
//-----------------------------------------------------------------------------
	CLASS_METHODS(CFileExtension, CBaseObject)
//-----------------------------------------------------------------------------
#if VSTGUI_RVALUE_REF_SUPPORT
	CFileExtension (CFileExtension&& ext) noexcept;
	CFileExtension& operator=(CFileExtension&& ext) noexcept;
#endif
protected:
	void init (UTF8StringPtr description, UTF8StringPtr extension, UTF8StringPtr mimeType, UTF8StringPtr uti);
	
	UTF8StringBuffer description;
	UTF8StringBuffer extension;
	UTF8StringBuffer mimeType;
	UTF8StringBuffer uti;
	int32_t macType;
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
CMessageResult MyClass::notify (CBaseObject* sender, IdStringPtr message)
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

#if VSTGUI_HAS_FUNCTIONAL
	typedef std::function<void(CNewFileSelector*)> CallbackFunc;
	bool run (CallbackFunc&& callback);
#endif
	bool run (CBaseObject* delegate);	///< the delegate will get a kSelectEndMessage throu the notify method where the sender is this CNewFileSelector object
	void cancel ();						///< cancel running the file selector
	bool runModal ();					///< run as modal dialog
	//@}

	//-----------------------------------------------------------------------------
	/// @name CFileSelector setup
	//-----------------------------------------------------------------------------
	//@{
	void setTitle (UTF8StringPtr title);						///< set title of file selector
	void setInitialDirectory (UTF8StringPtr path);				///< set initial directory (UTF8 string)
	void setDefaultSaveName (UTF8StringPtr name);				///< set initial save name (UTF8 string)
	void setDefaultExtension (const CFileExtension& extension);	///< set default file extension
	void setAllowMultiFileSelection (bool state);				///< set allow multi file selection (only valid for kSelectFile selector style)
	void addFileExtension (const CFileExtension& extension);	///< add a file extension
#if VSTGUI_RVALUE_REF_SUPPORT
	void addFileExtension (CFileExtension&& extension);			///< add a file extension
#endif
	//@}

	//-----------------------------------------------------------------------------
	/// @name CFileSelector result
	//-----------------------------------------------------------------------------
	//@{
	uint32_t getNumSelectedFiles () const;						///< get number of selected files
	UTF8StringPtr getSelectedFile (uint32_t index) const;		///< get selected file. Result is only valid as long as the instance of CNewFileSelector is valid.
	//@}

	static const CFileExtension& getAllFilesExtension ();		///< get the all files extension

	static IdStringPtr kSelectEndMessage;
//-----------------------------------------------------------------------------
	CLASS_METHODS_NOCOPY(CNewFileSelector, CBaseObject)
protected:
	CNewFileSelector (CFrame* frame = 0);
	~CNewFileSelector ();

	virtual bool runInternal (CBaseObject* delegate) = 0;
	virtual void cancelInternal () = 0;
	virtual bool runModalInternal () = 0;

	CFrame* frame;
	UTF8StringBuffer title;
	UTF8StringBuffer initialPath;
	UTF8StringBuffer defaultSaveName;
	const CFileExtension* defaultExtension;
	bool allowMultiFileSelection;

	typedef std::list<CFileExtension> FileExtensionList;
	FileExtensionList extensions;
	std::vector<UTF8StringBuffer> result;
};

} // namespace

#endif
