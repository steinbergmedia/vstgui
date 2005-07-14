#ifndef __cfileselector__
#define __cfileselector__

#include "vstgui.h"

BEGIN_NAMESPACE_VSTGUI

#ifndef __aeffectx__
struct VstFileSelect;
#endif

//-----------------------------------------------------------------------------
// CFileSelector Declaration
//!
//-----------------------------------------------------------------------------
class CFileSelector
{
public:
	CFileSelector (void* ptr);
	virtual ~CFileSelector ();

	long run (VstFileSelect *vstFileSelect);

protected:
	void* ptr;
	VstFileSelect *vstFileSelect;

	#if MAC
	static pascal void navEventProc (const NavEventCallbackMessage callBackSelector, NavCBRecPtr callBackParms, NavCallBackUserData callBackUD);
	static pascal Boolean navObjectFilterProc (AEDesc *theItem, void *info, void *callBackUD, NavFilterModes filterMode);
	#endif
};

#ifndef __aeffectx__
struct VstFileType
{
	VstFileType (char* _name, char *_macType, char *_dosType, char *_unixType = 0, char *_mimeType1 = 0, char *_mimeType2 = 0)
	{
		if (_name)
			strcpy (name, _name);
		if (_macType)
			strcpy (macType, _macType);
		if (_dosType)
			strcpy (dosType, _dosType);
		if (_unixType)
			strcpy (unixType, _unixType);
		if (_mimeType1)
			strcpy (mimeType1, _mimeType1);
		if (_mimeType2)
			strcpy (mimeType2, _mimeType2);
	}
	char name[128];
	char macType[8];
	char dosType[8];
	char unixType[8];
	char mimeType1[128];
	char mimeType2[128];
};

struct VstFileSelect
{
	long command;           // see enum kVstFileLoad....
	long type;              // see enum kVstFileType...

	long macCreator;        // optional: 0 = no creator

	long nbFileTypes;       // nb of fileTypes to used
	VstFileType *fileTypes; // list of fileTypes

	char title[1024];       // text display in the file selector's title

	char *initialPath;      // initial path

	char *returnPath;       // use with kVstFileLoad and kVstDirectorySelect
							// if null is passed, the host will allocated memory
							// the plugin should then called closeOpenFileSelector for freeing memory
	long sizeReturnPath; 

	char **returnMultiplePaths; // use with kVstMultipleFilesLoad
								// the host allocates this array. The plugin should then called closeOpenFileSelector for freeing memory
	long nbReturnPath;			// number of selected paths

	long reserved;				// reserved for host application
	char future[116];			// future use
};

enum {
	kVstFileLoad = 0,
	kVstFileSave,
	kVstMultipleFilesLoad,
	kVstDirectorySelect,

	kVstFileType = 0
};
#endif

END_NAMESPACE_VSTGUI

#endif
