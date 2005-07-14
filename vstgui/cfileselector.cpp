//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins : 
//
// Version 3.0       $Date: 2005-07-14 10:26:11 $ 
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

#if !PLUGGUI
#include "AudioEffectX.h"
#endif

#ifndef __cfileselector__
#include "cfileselector.h"
#endif

//-----------------------------------------------------------------------------
// CFileSelector Implementation
//-----------------------------------------------------------------------------
#define stringAnyType  "Any Type (*.*)"
#define stringAllTypes "All Types: ("
#define stringSelect   "Select"
#define stringCancel   "Cancel"
#define stringLookIn   "Look in"
#define kPathMax        1024

#if WINDOWS
#include <stdio.h>

static UINT APIENTRY SelectDirectoryHook (HWND hdlg, UINT message, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK SelectDirectoryButtonProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
WNDPROC fpOldSelectDirectoryButtonProc;
static UINT APIENTRY WinSaveHook (HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam);
static bool bFolderSelected;
static bool bDidCancel;
static char selDirPath[kPathMax];
#if PLUGGUI
	extern HINSTANCE ghInst;
	inline HINSTANCE GetInstance () { return ghInst; }
#else
	extern void* hInstance;
	inline HINSTANCE GetInstance () { return (HINSTANCE)hInstance; }
#endif
#endif

BEGIN_NAMESPACE_VSTGUI

//-----------------------------------------------------------------------------
CFileSelector::CFileSelector (void* ptr)
: ptr (ptr)
, vstFileSelect (0)
{}

//-----------------------------------------------------------------------------
CFileSelector::~CFileSelector ()
{
	if (vstFileSelect)
	{
		#if VST
		if (ptr && ((AudioEffectX*)ptr)->canHostDo ("closeFileSelector"))
			((AudioEffectX*)ptr)->closeFileSelector (vstFileSelect);
		else
		#endif
		{
			if (vstFileSelect->reserved == 1 && vstFileSelect->returnPath)
			{
				delete []vstFileSelect->returnPath;
				vstFileSelect->returnPath = 0;
				vstFileSelect->sizeReturnPath = 0;
			}
			if (vstFileSelect->returnMultiplePaths)
			{
				for (long i = 0; i < vstFileSelect->nbReturnPath; i++)
				{
					delete []vstFileSelect->returnMultiplePaths[i];
					vstFileSelect->returnMultiplePaths[i] = 0;
				}
				delete[] vstFileSelect->returnMultiplePaths;
				vstFileSelect->returnMultiplePaths = 0;
			}
		}
	}
}

//-----------------------------------------------------------------------------
long CFileSelector::run (VstFileSelect *vstFileSelect)
{
	this->vstFileSelect = vstFileSelect;
	vstFileSelect->nbReturnPath = 0;
	if (vstFileSelect->returnPath)
		vstFileSelect->returnPath[0] = 0;

	#if !PLUGGUI
	if (ptr
	#if MACX 
		&& vstFileSelect->command != kVstFileSave 
	#endif
		&& ((AudioEffectX*)ptr)->canHostDo ("openFileSelector") && ((AudioEffectX*)ptr)->canHostDo ("closeFileSelector"))
	{
		if (((AudioEffectX*)ptr)->openFileSelector (vstFileSelect))
			return vstFileSelect->nbReturnPath;
	}
	else
	#endif
	{
#if WINDOWS
		char filter[512];
		char filePathBuffer[kPathMax];
		strcpy (filePathBuffer, "");
		char* filePath = filePathBuffer;
		char fileName[kPathMax];
		strcpy (fileName, "");
		filter[0] = 0;
		filePath[0] = 0;
		fileName[0] = 0;

		//-----------------------------------------
		if (vstFileSelect->command == kVstFileLoad ||
			vstFileSelect->command == kVstMultipleFilesLoad ||
			vstFileSelect->command == kVstDirectorySelect)
		{
			char* multiBuffer = 0;
			if (vstFileSelect->command == kVstMultipleFilesLoad)
			{
				multiBuffer = new char [kPathMax * 100];
				strcpy (multiBuffer, "");
				filePath = multiBuffer;
			}

			if (vstFileSelect->command != kVstDirectorySelect) 
			{
				char allBuffer [kPathMax] = {0};
				char* p = filter;
				char* p2 = allBuffer;

				const char* ext;
				const char* extensions [100];
				long i, j, extCount = 0;
				char string[24];

				for (long ty = 0; ty < vstFileSelect->nbFileTypes; ty++)
				{
					for (i = 0; i < 2 ; i++)
					{				
						if (i == 0)
						{
							ext = vstFileSelect->fileTypes[ty].dosType;
						
							strcpy (p, vstFileSelect->fileTypes[ty].name);
							strcat (p, " (.");
							strcat (p, ext);
							strcat (p, ")");
							p += strlen (p) + 1;

							strcpy (string, "*.");
							strcat (string, ext);
							strcpy (p, string);
							p += strlen (p);	
						}
						else
						{
							if (!strcmp (vstFileSelect->fileTypes[ty].dosType, vstFileSelect->fileTypes[ty].unixType) || !strcmp (vstFileSelect->fileTypes[ty].unixType, ""))
								break; // for
							ext = vstFileSelect->fileTypes[ty].unixType;
							strcpy (string, ";*.");
							strcat (string, ext);
							strcpy (p, string);
							p += strlen (p);	
						}
						bool found = false;
						for (j = 0; j < extCount;j ++)
						{
							if (strcmp (ext, extensions [j]) == 0)
							{
								found = true;
								break;
							}
						}
						if (!found && extCount < 100)
							extensions [extCount++] = ext;
					}
					p ++;
				} // end for filetype
			
				if (extCount > 1)
				{
					for (i = 0; i < extCount ;i ++)
					{					
						ext = extensions [i];
						strcpy (string, "*.");
						strcat (string, ext);

						if (p2 != allBuffer)
						{
							strcpy (p2, ";");
							p2++;
						}
						strcpy (p2, string);
						p2 += strlen (p2);
					}

					// add the : All types
					strcpy (p, stringAllTypes);			
					strcat (p, allBuffer);
					strcat (p, ")");
					p += strlen (p) + 1;
					strcpy (p, allBuffer);
					p += strlen (p) + 1;			
				}

				strcpy (p, stringAnyType);
				p += strlen (p) + 1;
				strcpy (p, "*.*");
				p += strlen (p) + 1;

				*p++ = 0;
				*p++ = 0;
			}

			OPENFILENAME ofn = {0};
			ofn.lStructSize  = sizeof (OPENFILENAME);
			HWND owner = 0;
			#if !PLUGGUI
			if (((AudioEffectX*)ptr)->getEditor () && ((AEffGUIEditor*)((AudioEffectX*)ptr)->getEditor ())->getFrame ())
				owner = (HWND)((AEffGUIEditor*)((AudioEffectX*)ptr)->getEditor ())->getFrame ()->getSystemWindow ();
			#endif
			ofn.hwndOwner    = owner;
	
			if (vstFileSelect->command == kVstDirectorySelect) 
				ofn.lpstrFilter = "HideFileFilter\0*.___\0\0"; // to hide files
			else
				ofn.lpstrFilter  = filter[0] ? filter : 0;
			ofn.nFilterIndex = 1;
			ofn.lpstrCustomFilter = NULL;
			ofn.lpstrFile    = filePath;
			if (vstFileSelect->command == kVstMultipleFilesLoad)
				ofn.nMaxFile    = 100 * kPathMax - 1;
			else
				ofn.nMaxFile    = sizeof (filePathBuffer) - 1;

			ofn.lpstrFileTitle  = fileName;
			ofn.nMaxFileTitle   = 64;
			ofn.lpstrInitialDir = vstFileSelect->initialPath;
			ofn.lpstrTitle      = vstFileSelect->title;
			ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_EXPLORER | OFN_ENABLESIZING | OFN_ENABLEHOOK;
			if (vstFileSelect->command == kVstDirectorySelect)
			{
				ofn.Flags &= ~OFN_FILEMUSTEXIST;
				ofn.lpfnHook = SelectDirectoryHook;
			}

			if (vstFileSelect->command == kVstMultipleFilesLoad)
				ofn.Flags |= OFN_ALLOWMULTISELECT;
		
			vstFileSelect->nbReturnPath = 0;
			bDidCancel = true;

			if (GetOpenFileName (&ofn) || 
				((vstFileSelect->command == kVstDirectorySelect) && !bDidCancel && strlen (selDirPath) != 0))  
			{
				switch (vstFileSelect->command)
				{
				case kVstFileLoad:
					vstFileSelect->nbReturnPath = 1;
					if (!vstFileSelect->returnPath)
					{
						vstFileSelect->reserved = 1;
						vstFileSelect->returnPath = new char[strlen (ofn.lpstrFile) + 1];
						vstFileSelect->sizeReturnPath = (long)strlen (ofn.lpstrFile) + 1;			
					}
					strcpy (vstFileSelect->returnPath, ofn.lpstrFile);
					break;
				
				case kVstMultipleFilesLoad:
					{
					char string[kPathMax], directory[kPathMax];
					char *previous = ofn.lpstrFile;
					size_t len;
					bool dirFound = false;
					bool first = true;
					directory[0] = 0; // !!
					vstFileSelect->returnMultiplePaths = new char*[kPathMax];
					long i = 0;
					while (1)
					{
						if (*previous != 0)
						{   // something found
							if (!dirFound) 
							{
								dirFound = true;
								strcpy (directory, previous);
								len = strlen (previous) + 1;  // including 0
								previous += len;

								if (*previous == 0)
								{  // 1 selected file only		
									vstFileSelect->returnMultiplePaths[i] = new char [strlen (directory) + 1];
									strcpy (vstFileSelect->returnMultiplePaths[i++], directory);
								}
								else
								{
									if (directory[strlen (directory) - 1] != '\\')
										strcat (directory, "\\");
								}
							}
							else 
							{
								sprintf (string, "%s%s", directory, previous);
								len = strlen (previous) + 1;  // including 0
								previous += len;

								vstFileSelect->returnMultiplePaths[i] = new char [strlen (string) + 1];
								strcpy (vstFileSelect->returnMultiplePaths[i++], string);
							}
						}
						else
							break;
					}
					vstFileSelect->nbReturnPath = i;					
					} break;

				case kVstDirectorySelect: 
					vstFileSelect->nbReturnPath = 1;
					if (!vstFileSelect->returnPath)
					{
						vstFileSelect->reserved = 1;
						vstFileSelect->returnPath = new char[strlen (selDirPath) + 1];
						vstFileSelect->sizeReturnPath = (long)strlen (selDirPath) + 1;			
					}
					strcpy (vstFileSelect->returnPath, selDirPath);
				}
				if (multiBuffer)
					delete []multiBuffer;
				return vstFileSelect->nbReturnPath;
			}
			if (multiBuffer)
				delete []multiBuffer;
		}

		//-----------------------------------------
		else if (vstFileSelect->command == kVstFileSave)
		{
			char* p = filter;
			for (long ty = 0; ty < vstFileSelect->nbFileTypes; ty++)
			{
				const char* ext = vstFileSelect->fileTypes[ty].dosType;
				if (ext)
				{
					strcpy (p, vstFileSelect->fileTypes[ty].name);
					strcat (p, " (.");
					strcat (p, ext);
					strcat (p, ")");
					p += strlen (p) + 1;
	
					char string[24];
					strcpy (string, "*.");
					strcat (string, ext);
					strcpy (p, string);
					p += strlen (p) + 1;
				}
			}
			*p++ = 0;
			*p++ = 0;
		
			OPENFILENAME ofn = {0};
			ofn.lStructSize  = sizeof (OPENFILENAME);
			HWND owner = 0;
			#if !PLUGGUI
			if (((AudioEffectX*)ptr)->getEditor () && ((AEffGUIEditor*)((AudioEffectX*)ptr)->getEditor ())->getFrame ())
				owner = (HWND)((AEffGUIEditor*)((AudioEffectX*)ptr)->getEditor ())->getFrame ()->getSystemWindow ();
			#endif
			ofn.hwndOwner    = owner;
			ofn.hInstance    = GetInstance ();
			ofn.lpstrFilter = filter[0] ? filter : 0;
			ofn.nFilterIndex = 1;
			ofn.lpstrFile = filePath;
			ofn.lpstrCustomFilter = NULL;
			ofn.nMaxFile = sizeof (filePathBuffer) - 1;
			ofn.lpstrFileTitle = fileName;
			ofn.nMaxFileTitle = 64;
			ofn.lpstrInitialDir = vstFileSelect->initialPath;
			ofn.lpstrTitle = vstFileSelect->title;
			ofn.Flags = OFN_EXPLORER | OFN_ENABLESIZING | OFN_HIDEREADONLY | OFN_ENABLEHOOK;
			
			if (vstFileSelect->nbFileTypes >= 1)
				ofn.lpstrDefExt = vstFileSelect->fileTypes[0].dosType;
			
			// add a template view
			ofn.lCustData = (DWORD)0;
			ofn.lpfnHook = WinSaveHook;
			
			if (GetSaveFileName (&ofn))	
			{
				vstFileSelect->nbReturnPath = 1;
				if (!vstFileSelect->returnPath)
				{
					vstFileSelect->reserved = 1;
					vstFileSelect->returnPath = new char[strlen (ofn.lpstrFile) + 1];
					vstFileSelect->sizeReturnPath = (long)strlen (ofn.lpstrFile) + 1;			
				}
				strcpy (vstFileSelect->returnPath, ofn.lpstrFile);
			
				return vstFileSelect->nbReturnPath;
			}
			#if _DEBUG
			else
			{
				DWORD err = CommDlgExtendedError (); // for breakpoint
			}
			#endif
		}

#elif MAC
#if TARGET_API_MAC_CARBON
		#if MACX
		// new approach for supporting long filenames on mac os x is to use unix path mode
		// if vstFileSelect->future[0] is 1 on entry and 0 on exit the resulting paths are UTF8 encoded paths
		bool unixPathMode = (vstFileSelect->future[0] == 1);
		#endif
		NavEventUPP	eventUPP = NewNavEventUPP (CFileSelector::navEventProc);
		if (vstFileSelect->command == kVstFileSave)
		{
			NavDialogCreationOptions dialogOptions;
			NavGetDefaultDialogCreationOptions (&dialogOptions);
			dialogOptions.windowTitle = CFStringCreateWithCString (NULL, vstFileSelect->title[0] ? vstFileSelect->title : "Select a Destination", kCFStringEncodingUTF8);
			CFStringRef defSaveName = 0;
			#if MACX
			if (unixPathMode && vstFileSelect->initialPath)
			{
				char* name = strrchr (vstFileSelect->initialPath, '/');
				if (name && name[1] != 0)
				{
					defSaveName = dialogOptions.saveFileName = CFStringCreateWithCString (NULL, name+1, kCFStringEncodingUTF8);
					name[0] = 0;
					dialogOptions.optionFlags |= kNavPreserveSaveFileExtension;
				}
				else if (name == 0)
				{
					defSaveName = dialogOptions.saveFileName = CFStringCreateWithCString (NULL, vstFileSelect->initialPath, kCFStringEncodingUTF8);
					dialogOptions.optionFlags |= kNavPreserveSaveFileExtension;
					vstFileSelect->initialPath = 0;
				}
			}
			else
			#endif
			if (vstFileSelect->initialPath && ((FSSpec*)vstFileSelect->initialPath)->name)
			{
				FSSpec* defaultSpec = (FSSpec*)vstFileSelect->initialPath;
				defSaveName = CFStringCreateWithPascalString (NULL, defaultSpec->name, kCFStringEncodingASCII);
				if (defSaveName)
				{
					dialogOptions.saveFileName = defSaveName;
					dialogOptions.optionFlags |= kNavPreserveSaveFileExtension;
				}
				*defaultSpec->name = 0;
			}
			NavDialogRef dialogRef;
			if (NavCreatePutFileDialog (&dialogOptions, NULL, kNavGenericSignature, eventUPP, this, &dialogRef) == noErr) 
			{
			    AEDesc defaultLocation;   
			    AEDesc* defLocPtr = 0;   
				if (vstFileSelect->initialPath)
				{
					#if MACX
					if (unixPathMode)
					{
						FSRef fsRef;
						if (FSPathMakeRef ((const unsigned char*)vstFileSelect->initialPath, &fsRef, NULL) == noErr)
						{
				            if (AECreateDesc (typeFSRef, &fsRef, sizeof(FSRef), &defaultLocation) == noErr)
				                defLocPtr = &defaultLocation;
						}
					}
					else
					#endif
					{
						FSSpec* defaultSpec = (FSSpec*)vstFileSelect->initialPath;
				        if (defaultSpec->parID && defaultSpec->vRefNum)
				        {
				            if (AECreateDesc (typeFSS, defaultSpec, sizeof(FSSpec), &defaultLocation) == noErr)
				                defLocPtr = &defaultLocation;
						}
					}
				}
		        if (defLocPtr)
		            NavCustomControl (dialogRef, kNavCtlSetLocation, (void*)defLocPtr);
				NavDialogRun (dialogRef);

				if (defLocPtr)
		            AEDisposeDesc (defLocPtr);

				NavReplyRecord navReply;
				if (NavDialogGetReply (dialogRef, &navReply) == noErr)
				{
					FSRef parentFSRef;
					AEKeyword theAEKeyword;
					DescType typeCode;
					Size actualSize;
			        // get the FSRef referring to the parent directory
				    if (AEGetNthPtr(&navReply.selection, 1, typeFSRef,
		        		&theAEKeyword, &typeCode, &parentFSRef, sizeof(FSRef), &actualSize) == noErr)
					{
						#if MACX
						if (unixPathMode)
						{
							bool success = true;
							vstFileSelect->nbReturnPath = 1;
							if (vstFileSelect->returnPath == 0)
							{
								vstFileSelect->reserved = 1;
								vstFileSelect->returnPath = new char [PATH_MAX];
							}
							if (FSRefMakePath (&parentFSRef, (unsigned char*)vstFileSelect->returnPath, PATH_MAX) == noErr)
							{
								char saveFileName [PATH_MAX];
								if (CFStringGetCString (navReply.saveFileName, saveFileName, PATH_MAX, kCFStringEncodingUTF8))
								{
									strcat (vstFileSelect->returnPath, "/");
									strcat (vstFileSelect->returnPath, saveFileName);
									vstFileSelect->future[0] = 0;
								}
								else
									success = false;
							}
							else
								success = false;
							if (!success && vstFileSelect->reserved)
							{
								vstFileSelect->nbReturnPath = 0;
								delete [] vstFileSelect->returnPath;
							}
						}
						else
						#endif
						{
							FSSpec spec;
							FSCatalogInfoBitmap infoBitmap = kFSCatInfoNone;
							FSGetCatalogInfo (&parentFSRef, infoBitmap, NULL, NULL, &spec, NULL);
							CInfoPBRec pbRec = {0};	
							pbRec.dirInfo.ioDrDirID = spec.parID;
							pbRec.dirInfo.ioVRefNum = spec.vRefNum;
							pbRec.dirInfo.ioNamePtr = spec.name;
							if (PBGetCatInfoSync (&pbRec) == noErr)
							{
								spec.parID = pbRec.dirInfo.ioDrDirID;
								// the cfstring -> pascalstring can fail if the filename length > 63 (FSSpec sucks)
								if (CFStringGetPascalString (navReply.saveFileName, (unsigned char*)&spec.name, sizeof (spec.name), kCFStringEncodingASCII))
								{
									vstFileSelect->nbReturnPath = 1;
									if (!vstFileSelect->returnPath)
									{
										vstFileSelect->reserved = 1;
										vstFileSelect->returnPath = new char [sizeof (FSSpec)];
									}
									memcpy (vstFileSelect->returnPath, &spec, sizeof (FSSpec));
								}
							}
						}
					}
					NavDisposeReply (&navReply);
				}
				if (defSaveName)
					CFRelease (defSaveName);
				NavDialogDispose (dialogRef);
				DisposeNavEventUPP (eventUPP);
				return vstFileSelect->nbReturnPath;
			}
			if (defSaveName)
				CFRelease (defSaveName);
		}
		else if (vstFileSelect->command == kVstDirectorySelect)
		{
			NavDialogCreationOptions dialogOptions;
			NavGetDefaultDialogCreationOptions (&dialogOptions);
			dialogOptions.windowTitle = CFStringCreateWithCString (NULL, vstFileSelect->title[0] ? vstFileSelect->title : "Select Directory", kCFStringEncodingUTF8);
			NavDialogRef dialogRef;
			if (NavCreateChooseFolderDialog (&dialogOptions, eventUPP, NULL, this, &dialogRef) == noErr)
			{
			    AEDesc defaultLocation;   
			    AEDesc* defLocPtr = 0;   
				if (vstFileSelect->initialPath)
				{
					#if MACX
					if (unixPathMode)
					{
						FSRef fsRef;
						if (FSPathMakeRef ((const unsigned char*)vstFileSelect->initialPath, &fsRef, NULL) == noErr)
						{
				            if (AECreateDesc (typeFSRef, &fsRef, sizeof(FSRef), &defaultLocation) == noErr)
				                defLocPtr = &defaultLocation;
						}
					}
					else
					#endif
					{
						FSSpec* defaultSpec = (FSSpec*)vstFileSelect->initialPath;
				        if (defaultSpec->parID && defaultSpec->vRefNum)       
				            if (AECreateDesc (typeFSS, defaultSpec, sizeof(FSSpec), &defaultLocation) == noErr)
				                defLocPtr = &defaultLocation;
			        }
				}
		        if (defLocPtr)
		            NavCustomControl (dialogRef, kNavCtlSetLocation, (void*)defLocPtr);
				NavDialogRun (dialogRef);
				if (defLocPtr)
		            AEDisposeDesc (defLocPtr);
				NavReplyRecord navReply;
				if (NavDialogGetReply (dialogRef, &navReply) == noErr)
				{
					FSRef parentFSRef;
					AEKeyword theAEKeyword;
					DescType typeCode;
					Size actualSize;
				    if (AEGetNthPtr(&navReply.selection, 1, typeFSRef,
		        		&theAEKeyword, &typeCode, &parentFSRef, sizeof(FSRef), &actualSize) == noErr)
					{
						#if MACX
						if (unixPathMode)
						{
							vstFileSelect->nbReturnPath = 1;
							if (vstFileSelect->returnPath == 0)
							{
								vstFileSelect->reserved = 1;
								vstFileSelect->returnPath = new char [PATH_MAX];
							}
							if (FSRefMakePath (&parentFSRef, (unsigned char*)vstFileSelect->returnPath, PATH_MAX) != noErr)
							{
								vstFileSelect->nbReturnPath = 0;
								if (vstFileSelect->reserved)
									delete [] vstFileSelect->returnPath;
							}
							else
								vstFileSelect->future[0] = 0;
						}
						else
						#endif
						{
							FSSpec spec;
							FSCatalogInfoBitmap infoBitmap = kFSCatInfoNone;
							FSGetCatalogInfo (&parentFSRef, infoBitmap, NULL, NULL, &spec, NULL);
							vstFileSelect->nbReturnPath = 1;
							if (!vstFileSelect->returnPath)
							{
								vstFileSelect->reserved = 1;
								vstFileSelect->returnPath = new char [sizeof (FSSpec)];
							}
							memcpy (vstFileSelect->returnPath, &spec, sizeof (FSSpec));
						}
					}
					
					NavDisposeReply (&navReply);
				}
				NavDialogDispose (dialogRef);
				DisposeNavEventUPP (eventUPP);
				return vstFileSelect->nbReturnPath;
			}
		}
		else // FileLoad
		{
			NavDialogCreationOptions dialogOptions;
			NavGetDefaultDialogCreationOptions (&dialogOptions);
			if (vstFileSelect->command == kVstFileLoad)
			{
				dialogOptions.windowTitle = CFStringCreateWithCString (NULL, vstFileSelect->title[0] ? vstFileSelect->title : "Select a File to Open", kCFStringEncodingUTF8);
				dialogOptions.optionFlags &= ~kNavAllowMultipleFiles;
			}
			else
			{
				dialogOptions.windowTitle = CFStringCreateWithCString (NULL, vstFileSelect->title[0] ? vstFileSelect->title : "Select Files to Open", kCFStringEncodingUTF8);
				dialogOptions.optionFlags |= kNavAllowMultipleFiles;
			}
			NavObjectFilterUPP objectFilterUPP = NewNavObjectFilterUPP (CFileSelector::navObjectFilterProc);
			NavDialogRef dialogRef;
			if (NavCreateGetFileDialog (&dialogOptions, NULL, eventUPP, NULL, objectFilterUPP, this, &dialogRef) == noErr)
			{
			    AEDesc defaultLocation;   
			    AEDesc* defLocPtr = 0;   
				if (vstFileSelect->initialPath)
				{
					#if MACX
					if (unixPathMode)
					{
						FSRef fsRef;
						if (FSPathMakeRef ((const unsigned char*)vstFileSelect->initialPath, &fsRef, NULL) == noErr)
						{
				            if (AECreateDesc (typeFSRef, &fsRef, sizeof(FSRef), &defaultLocation) == noErr)
				                defLocPtr = &defaultLocation;
						}
					}
					else
					#endif
					{
						FSSpec* defaultSpec = (FSSpec*)vstFileSelect->initialPath;
				        if (defaultSpec->parID && defaultSpec->vRefNum)       
				            if (AECreateDesc (typeFSS, defaultSpec, sizeof(FSSpec), &defaultLocation) == noErr)
				                defLocPtr = &defaultLocation;
			        }
				}
		        if (defLocPtr)
		            NavCustomControl (dialogRef, kNavCtlSetLocation, (void*)defLocPtr);

				NavDialogRun (dialogRef);

				if (defLocPtr)
		            AEDisposeDesc (defLocPtr);

				NavReplyRecord navReply;
				if (NavDialogGetReply (dialogRef, &navReply) == noErr)
				{
					FSRef parentFSRef;
					AEKeyword theAEKeyword;
					DescType typeCode;
					Size actualSize;
					if (vstFileSelect->command == kVstFileLoad)
					{
					    if (AEGetNthPtr(&navReply.selection, 1, typeFSRef,
			        		&theAEKeyword, &typeCode, &parentFSRef, sizeof(FSRef), &actualSize) == noErr)
						{
							#if MACX
							if (unixPathMode)
							{
								vstFileSelect->nbReturnPath = 1;
								if (vstFileSelect->returnPath == 0)
								{
									vstFileSelect->reserved = 1;
									vstFileSelect->returnPath = new char [PATH_MAX];
								}
								if (FSRefMakePath (&parentFSRef, (unsigned char*)vstFileSelect->returnPath, PATH_MAX) != noErr)
								{
									vstFileSelect->nbReturnPath = 0;
									if (vstFileSelect->reserved)
										delete [] vstFileSelect->returnPath;
								}
								else
									vstFileSelect->future[0] = 0;
							}
							else
							#endif
							{
								FSSpec spec;
								FSCatalogInfoBitmap infoBitmap = kFSCatInfoNone;
								FSGetCatalogInfo (&parentFSRef, infoBitmap, NULL, NULL, &spec, NULL);
								vstFileSelect->nbReturnPath = 1;
								if (!vstFileSelect->returnPath)
								{
									vstFileSelect->reserved = 1;
									vstFileSelect->returnPath = new char [sizeof (FSSpec)];
								}
								memcpy (vstFileSelect->returnPath, &spec, sizeof (FSSpec));
							}
						}
					}
					else
					{
						AECountItems (&navReply.selection, &vstFileSelect->nbReturnPath);
						vstFileSelect->returnMultiplePaths = new char* [vstFileSelect->nbReturnPath];
						int index = 1;
					    while (AEGetNthPtr(&navReply.selection, index++, typeFSRef,
			        		&theAEKeyword, &typeCode, &parentFSRef, sizeof(FSRef), &actualSize) == noErr)
						{
							#if MACX
							if (unixPathMode)
							{
								vstFileSelect->returnMultiplePaths[index-2] = new char[PATH_MAX];
								FSRefMakePath (&parentFSRef, (unsigned char*)vstFileSelect->returnMultiplePaths[index-2], PATH_MAX);
								vstFileSelect->future[0] = 0;
							}
							else
							#endif
							{
								FSSpec spec;
								FSCatalogInfoBitmap infoBitmap = kFSCatInfoNone;
								FSGetCatalogInfo (&parentFSRef, infoBitmap, NULL, NULL, &spec, NULL);
								vstFileSelect->returnMultiplePaths[index-2] = new char[sizeof (FSSpec)];
								memcpy (vstFileSelect->returnMultiplePaths[index-2], &spec, sizeof (FSSpec));
							}
						}
					}
				}
				DisposeNavObjectFilterUPP (objectFilterUPP);
				DisposeNavEventUPP (eventUPP);
				NavDialogDispose (dialogRef);
				return vstFileSelect->nbReturnPath;
			}
			DisposeNavObjectFilterUPP (objectFilterUPP);
		}
		DisposeNavEventUPP (eventUPP);
#else
		StandardFileReply reply;
		if (vstFileSelect->command == kVstFileSave)
		{
			unsigned char defName[64];
			defName[0] = 0;
			StandardPutFile ("\pSelect a Destination", defName, &reply);
			if (reply.sfGood && reply.sfFile.name[0] != 0)
			{
				if (!vstFileSelect->returnPath)
				{
					vstFileSelect->reserved = 1;
					vstFileSelect->returnPath = new char [301];
				}
				memcpy (vstFileSelect->returnPath, &reply.sfFile, 300);
				vstFileSelect->nbReturnPath = 1;
				return 1;
			}
		}

		else if (vstFileSelect->command == kVstDirectorySelect) 
		{
		#if USENAVSERVICES
			if (NavServicesAvailable ())
			{
				NavReplyRecord navReply;
				NavDialogOptions dialogOptions;
				short ret = false;
				AEDesc defLoc;
				defLoc.descriptorType = typeFSS;
				defLoc.dataHandle = NewHandle (sizeof (FSSpec));
				FSSpec	finalFSSpec;
				finalFSSpec.parID   = 0;	// *dirID;
				finalFSSpec.vRefNum = 0;	// *volume;
				finalFSSpec.name[0] = 0;

				NavGetDefaultDialogOptions (&dialogOptions);
				dialogOptions.dialogOptionFlags &= ~kNavAllowMultipleFiles;
				dialogOptions.dialogOptionFlags |= kNavSelectDefaultLocation;
				strcpy ((char* )dialogOptions.message, "Select Directory");
				c2pstr ((char* )dialogOptions.message);
				NavChooseFolder (&defLoc, &navReply, &dialogOptions, 0 /* eventUPP */, 0, 0);
				DisposeHandle (defLoc.dataHandle);
				
				AEDesc 	resultDesc;	
				AEKeyword keyword;
				resultDesc.dataHandle = 0L;

				if (navReply.validRecord && AEGetNthDesc (&navReply.selection, 1, typeFSS, &keyword, &resultDesc) == noErr)
				{
					ret = true;
					vstFileSelect->nbReturnPath = 1;
					if (!vstFileSelect->returnPath)
					{
						vstFileSelect->reserved = 1;
						vstFileSelect->returnPath = new char [sizeof (FSSpec)];
					}
					memcpy (vstFileSelect->returnPath, *resultDesc.dataHandle, sizeof (FSSpec));
				}
				NavDisposeReply (&navReply);
				return vstFileSelect->nbReturnPath;
			}
			else
		#endif
			{
				// Can't select a Folder; the Application does not support it, and Navigational Services are not available...
				return 0;
			}
		}

		else
		{
			SFTypeList typelist;
			long numFileTypes = vstFileSelect->nbFileTypes;
			//seem not to work... if (numFileTypes <= 0)
			{
				numFileTypes = -1;	// all files
				typelist[0] = 'AIFF';
			}
			/*else
			{
				if (numFileTypes > 4)
					numFileTypes = 4;
				for (long i = 0; i < numFileTypes; i++)
					memcpy (&typelist[i], vstFileSelect->fileTypes[i].macType, 4);
			}*/
			StandardGetFile (0L, numFileTypes, typelist, &reply);
			if (reply.sfGood)
			{
				if (!vstFileSelect->returnPath)
				{
					vstFileSelect->reserved = 1;
					vstFileSelect->returnPath = new char [301];
				}
				memcpy (vstFileSelect->returnPath, &reply.sfFile, 300);
				vstFileSelect->nbReturnPath = 1;
				return 1;
			}
		}
#endif // TARGET_API_MAC_CARBON
#else
		//CAlert::alert ("The current Host application doesn't support FileSelector !", "Warning");
#endif
	}
	return 0;
}

#if MAC && TARGET_API_MAC_CARBON
//-----------------------------------------------------------------------------
pascal void CFileSelector::navEventProc (const NavEventCallbackMessage callBackSelector, NavCBRecPtr callBackParms, NavCallBackUserData callBackUD) 
{
	CFileSelector* fs = (CFileSelector*)callBackUD;
	switch (callBackSelector)
	{
		case kNavCBEvent:
		{
			#if !PLUGGUI
			AudioEffectX* effect = (AudioEffectX*)fs->ptr;
			if (effect && callBackParms->eventData.eventDataParms.event->what == nullEvent)
				effect->masterIdle ();
			#endif
			break;
		}
	}
}

//-----------------------------------------------------------------------------
pascal Boolean CFileSelector::navObjectFilterProc (AEDesc *theItem, void *info, void *callBackUD, NavFilterModes filterMode)
{
    Boolean result = false;
	CFileSelector* fs = (CFileSelector*)callBackUD;
    NavFileOrFolderInfo *theInfo = (NavFileOrFolderInfo*)info;

	if (theInfo->isFolder || fs->vstFileSelect->nbFileTypes == 0)
		result = true;
	else
	{
	    FSRef ref;
		AECoerceDesc (theItem, typeFSRef, theItem);
		if (AEGetDescData (theItem, &ref, sizeof (FSRef)) == noErr)
		{
			LSItemInfoRecord infoRecord;
			if (LSCopyItemInfoForRef (&ref, kLSRequestExtension | kLSRequestTypeCreator, &infoRecord) == noErr)
			{
				char extension [128];
				extension[0] = 0;
				if (infoRecord.extension)
					CFStringGetCString (infoRecord.extension, extension, 128, kCFStringEncodingUTF8);
				for (long i = 0; i < fs->vstFileSelect->nbFileTypes; i++)
				{
					VstFileType* ft = &fs->vstFileSelect->fileTypes[i];
					if ((OSType)ft->macType == infoRecord.filetype)
					{
						result = true;
						break;
					}
					else if (infoRecord.extension)
					{
						if (!strcasecmp (extension, ft->unixType) || !strcasecmp (extension, ft->dosType))
						{
							result = true;
							break;
						}
					}
				}
				if (infoRecord.extension)
					CFRelease (infoRecord.extension);
			}
		}
	}
	return result;
}
#endif

END_NAMESPACE_VSTGUI

#if WINDOWS
#include <dlgs.h>
//-----------------------------------------------------------------------------
UINT APIENTRY SelectDirectoryHook (HWND hdlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_NOTIFY: 
	{
		OFNOTIFY *lpon = (OFNOTIFY *)lParam;
	
		switch (lpon->hdr.code)
		{
		case CDN_FILEOK:
			CommDlg_OpenSave_GetFolderPath (GetParent (hdlg), selDirPath, kPathMax);
			bDidCancel = false;
			break;
		
		case CDN_INITDONE: {
			#define HIDE_ITEMS 4
			int  i;
			UINT hide_items[HIDE_ITEMS] = {edt1, stc3, cmb1, stc2};	

			for (i = 0; i < HIDE_ITEMS; i++)
				CommDlg_OpenSave_HideControl (GetParent (hdlg), hide_items[i]);
			
			CommDlg_OpenSave_SetControlText (GetParent (hdlg), stc4, (char*)(const char*)stringLookIn);
			CommDlg_OpenSave_SetControlText (GetParent (hdlg), IDOK, (char*)(const char*)stringSelect);
			CommDlg_OpenSave_SetControlText (GetParent (hdlg), IDCANCEL, (char*)(const char*)stringCancel);
		} break;
		}
	} break;

	case WM_INITDIALOG:
		fpOldSelectDirectoryButtonProc = /*(FARPROC)*/(WNDPROC)SetWindowLongPtr (
					GetDlgItem (GetParent (hdlg), IDOK), 
					GWLP_WNDPROC, (LONG_PTR)SelectDirectoryButtonProc);
		break;
		
	case WM_DESTROY:
		SetWindowLong (GetDlgItem (GetParent (hdlg), IDOK), 
				GWLP_WNDPROC, (LONG_PTR)fpOldSelectDirectoryButtonProc);
	}
	return false;
}

//-----------------------------------------------------------------------------
LRESULT CALLBACK SelectDirectoryButtonProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_SETTEXT: 
		if (! (strcmp ((char *)lParam, stringSelect) == 0))
			return false;
		break;
	
	case WM_LBUTTONUP:
	case WM_RBUTTONUP: {
		char mode[256];
		GetWindowText (hwnd, mode, 256);
		if (!strcmp (mode, stringSelect))
		{
			bFolderSelected = true;
			char oldDirPath[kPathMax];
			CommDlg_OpenSave_GetFolderPath (GetParent (hwnd), oldDirPath, kPathMax);
			// you need a lot of tricks to get name of currently selected folder:
			// the following call of the original windows procedure causes the
			// selected folder to open and after that you can retrieve its name
			// by calling ..._GetFolderPath (...)
			CallWindowProc ((WNDPROC)fpOldSelectDirectoryButtonProc, hwnd, message, wParam, lParam);
			CommDlg_OpenSave_GetFolderPath (GetParent (hwnd), selDirPath, kPathMax);

			if (1) // consumers like it like this
			{
				if (strcmp (oldDirPath, selDirPath) == 0 || selDirPath [0] == 0)
				{
					// the same folder as the old one, means nothing selected: close
					bFolderSelected = true;
					bDidCancel = false;
					PostMessage (GetParent (hwnd), WM_CLOSE, 0, 0);
					return false;
				}
				else
				{
					// another folder is selected: browse into it
					bFolderSelected = false;
					return true;
				}
			}
			else // original code
			{
				if (strcmp (oldDirPath, selDirPath) == 0 || selDirPath [0] == 0)
				{
					// the same folder as the old one, means nothing selected: stay open
					bFolderSelected = false;
					return true;
				}
			}
		}

		bDidCancel = false;
		PostMessage (GetParent (hwnd), WM_CLOSE, 0, 0); 
		return false;
	} break;
	} // end switch

	return CallWindowProc ((WNDPROC)fpOldSelectDirectoryButtonProc, hwnd, message, wParam, lParam);
}

//-----------------------------------------------------------------------------
static void showPathInWindowTitle (HWND hParent, LPOFNOTIFY lpon)
{
	#define WINDOWTEXTSIZE 260 + 64
	OPENFILENAME *ofn = lpon->lpOFN;
	char text[WINDOWTEXTSIZE];
	char *p;
	size_t len;

	// Put the path into the Window Title
	if (lpon->lpOFN->lpstrTitle)
		strcpy (text, lpon->lpOFN->lpstrTitle);
	else
	{
		char *pp;

		GetWindowText (hParent, text, WINDOWTEXTSIZE);
		pp = strchr (text, '-');
		if (pp)
			*--pp = 0;
	}

	p = strcat (text, " - [");
	p = text;
	len = strlen (text); 
	p += len;
	len = WINDOWTEXTSIZE - len - 2;
	CommDlg_OpenSave_GetFolderPath (hParent, p, len);
	strcat (text, "]");
	SetWindowText (hParent, text);
}

//------------------------------------------------------------------------
UINT APIENTRY WinSaveHook (HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_NOTIFY: {
		LPOFNOTIFY lpon = (LPOFNOTIFY)lParam; 
		if (!lpon)
			break;

		switch (lpon->hdr.code)
		{
		case CDN_FOLDERCHANGE: 
			showPathInWindowTitle (GetParent (hdlg), lpon);
			break;
		}
	} break;
	} // end switch

	return 0;
}
#endif

//-----------------------------------------------------------------------------
