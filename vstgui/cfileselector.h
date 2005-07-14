//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins : 
//
// Version 3.0       $Date: 2005-07-14 10:20:26 $ 
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// © 2004, Steinberg Media Technologies, All Rights Reserved
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
