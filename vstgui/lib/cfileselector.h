// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#ifndef __cfileselector__
#define __cfileselector__

#include "vstguifwd.h"
#include "cstring.h"
#include <list>
#include <vector>
#include <functional>

namespace VSTGUI {

//-----------------------------------------------------------------------------
// CFileExtension Declaration
//! @brief file extension description
//-----------------------------------------------------------------------------
class CFileExtension
{
public:
	CFileExtension (const UTF8String& description, const UTF8String& extension, const UTF8String& mimeType = "", int32_t macType = 0, const UTF8String& uti = "");
	CFileExtension (const CFileExtension& ext);
	~CFileExtension () noexcept;

	const UTF8String& getDescription () const { return description; }
	const UTF8String& getExtension () const { return extension; }
	const UTF8String& getMimeType () const { return mimeType; }
	const UTF8String& getUTI () const { return uti; }
	int32_t getMacType () const { return macType; }

	bool operator== (const CFileExtension& ext) const;
//-----------------------------------------------------------------------------
	CFileExtension (CFileExtension&& ext) noexcept;
	CFileExtension& operator=(CFileExtension&& ext) noexcept;
protected:
	void init (const UTF8String& description, const UTF8String& extension, const UTF8String& mimeType, const UTF8String& uti);
	
	UTF8String description;
	UTF8String extension;
	UTF8String mimeType;
	UTF8String uti;
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
	static CNewFileSelector* create (CFrame* parent = nullptr, Style style = kSelectFile); ///< create a new instance

	using CallbackFunc = std::function<void(CNewFileSelector*)>;
	bool run (CallbackFunc&& callback);
	bool run (CBaseObject* delegate);	///< the delegate will get a kSelectEndMessage throu the notify method where the sender is this CNewFileSelector object
	void cancel ();						///< cancel running the file selector
	bool runModal ();					///< run as modal dialog
	//@}

	//-----------------------------------------------------------------------------
	/// @name CFileSelector setup
	//-----------------------------------------------------------------------------
	//@{
	void setTitle (const UTF8String& title);					///< set title of file selector
	void setInitialDirectory (const UTF8String& path);			///< set initial directory (UTF8 string)
	void setDefaultSaveName (const UTF8String& name);			///< set initial save name (UTF8 string)
	void setDefaultExtension (const CFileExtension& extension);	///< set default file extension
	void setAllowMultiFileSelection (bool state);				///< set allow multi file selection (only valid for kSelectFile selector style)
	void addFileExtension (const CFileExtension& extension);	///< add a file extension
	void addFileExtension (CFileExtension&& extension);			///< add a file extension
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
	explicit CNewFileSelector (CFrame* frame = nullptr);
	~CNewFileSelector () noexcept override;

	virtual bool runInternal (CBaseObject* delegate) = 0;
	virtual void cancelInternal () = 0;
	virtual bool runModalInternal () = 0;

	CFrame* frame;
	UTF8String title;
	UTF8String initialPath;
	UTF8String defaultSaveName;
	const CFileExtension* defaultExtension;
	bool allowMultiFileSelection;

	using FileExtensionList = std::list<CFileExtension>;
	FileExtensionList extensions;
	std::vector<UTF8String> result;
};

} // namespace

#endif
