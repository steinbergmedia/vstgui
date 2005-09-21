# Microsoft Developer Studio Project File - Name="drawtest" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=drawtest - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "drawtest.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "drawtest.mak" CFG="drawtest - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "drawtest - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "drawtest - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "drawtest - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DRAWTEST_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DRAWTEST_EXPORTS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 gdiplus.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib GdiPlus.lib /nologo /dll /machine:I386

!ELSEIF  "$(CFG)" == "drawtest - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DRAWTEST_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\vstsdk2.3\\" /I "..\..\vstgui\\" /I "..\..\..\sdk\libpng" /I "..\..\..\sdk\zlib" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DRAWTEST_EXPORTS" /D USE_LIBPNG=1 /D DEBUG=1 /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib GdiPlus.lib /nologo /dll /debug /machine:I386 /out:"C:\Program Files\Steinberg\VstPlugins\drawtest.dll" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "drawtest - Win32 Release"
# Name "drawtest - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "vstgui"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\vstgui\aeffguieditor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\vstgui\cfileselector.cpp
# End Source File
# Begin Source File

SOURCE=..\..\vstgui\cscrollview.cpp
# End Source File
# Begin Source File

SOURCE=..\..\vstgui\ctabview.cpp
# End Source File
# Begin Source File

SOURCE=..\..\vstgui\vstcontrols.cpp
# End Source File
# Begin Source File

SOURCE=..\..\vstgui\vstgui.cpp
# End Source File
# End Group
# Begin Group "vstsdk"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\vstsdk2.3\AudioEffect.cpp
# End Source File
# Begin Source File

SOURCE=..\vstsdk2.3\audioeffectx.cpp
# End Source File
# End Group
# Begin Group "libpng"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\sdk\libpng\png.c
# End Source File
# Begin Source File

SOURCE=..\..\..\sdk\libpng\png.h
# End Source File
# Begin Source File

SOURCE=..\..\..\sdk\libpng\pngasmrd.h
# End Source File
# Begin Source File

SOURCE=..\..\..\sdk\libpng\pngconf.h
# End Source File
# Begin Source File

SOURCE=..\..\..\sdk\libpng\pngerror.c
# End Source File
# Begin Source File

SOURCE=..\..\..\sdk\libpng\pnggccrd.c
# End Source File
# Begin Source File

SOURCE=..\..\..\sdk\libpng\pngget.c
# End Source File
# Begin Source File

SOURCE=..\..\..\sdk\libpng\pngmem.c
# End Source File
# Begin Source File

SOURCE=..\..\..\sdk\libpng\pngpread.c
# End Source File
# Begin Source File

SOURCE=..\..\..\sdk\libpng\pngread.c
# End Source File
# Begin Source File

SOURCE=..\..\..\sdk\libpng\pngrio.c
# End Source File
# Begin Source File

SOURCE=..\..\..\sdk\libpng\pngrtran.c
# End Source File
# Begin Source File

SOURCE=..\..\..\sdk\libpng\pngrutil.c
# End Source File
# Begin Source File

SOURCE=..\..\..\sdk\libpng\pngset.c
# End Source File
# Begin Source File

SOURCE=..\..\..\sdk\libpng\pngtrans.c
# End Source File
# Begin Source File

SOURCE=..\..\..\sdk\libpng\pngvcrd.c
# End Source File
# Begin Source File

SOURCE=..\..\..\sdk\libpng\pngwio.c
# End Source File
# Begin Source File

SOURCE=..\..\..\sdk\libpng\pngwrite.c
# End Source File
# Begin Source File

SOURCE=..\..\..\sdk\libpng\pngwtran.c
# End Source File
# Begin Source File

SOURCE=..\..\..\sdk\libpng\pngwutil.c
# End Source File
# End Group
# Begin Group "zlib"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\sdk\zlib\adler32.c
# End Source File
# Begin Source File

SOURCE=..\..\..\sdk\zlib\compress.c
# End Source File
# Begin Source File

SOURCE=..\..\..\sdk\zlib\crc32.c
# End Source File
# Begin Source File

SOURCE=..\..\..\sdk\zlib\crc32.h
# End Source File
# Begin Source File

SOURCE=..\..\..\sdk\zlib\deflate.c
# End Source File
# Begin Source File

SOURCE=..\..\..\sdk\zlib\deflate.h
# End Source File
# Begin Source File

SOURCE=..\..\..\sdk\zlib\gzio.c
# End Source File
# Begin Source File

SOURCE=..\..\..\sdk\zlib\infback.c
# End Source File
# Begin Source File

SOURCE=..\..\..\sdk\zlib\inffast.c
# End Source File
# Begin Source File

SOURCE=..\..\..\sdk\zlib\inffast.h
# End Source File
# Begin Source File

SOURCE=..\..\..\sdk\zlib\inffixed.h
# End Source File
# Begin Source File

SOURCE=..\..\..\sdk\zlib\inflate.c
# End Source File
# Begin Source File

SOURCE=..\..\..\sdk\zlib\inflate.h
# End Source File
# Begin Source File

SOURCE=..\..\..\sdk\zlib\inftrees.c
# End Source File
# Begin Source File

SOURCE=..\..\..\sdk\zlib\inftrees.h
# End Source File
# Begin Source File

SOURCE=..\..\..\sdk\zlib\trees.c
# End Source File
# Begin Source File

SOURCE=..\..\..\sdk\zlib\trees.h
# End Source File
# Begin Source File

SOURCE=..\..\..\sdk\zlib\uncompr.c
# End Source File
# Begin Source File

SOURCE=..\..\..\sdk\zlib\zconf.h
# End Source File
# Begin Source File

SOURCE=..\..\..\sdk\zlib\zlib.h
# End Source File
# Begin Source File

SOURCE=..\..\..\sdk\zlib\zutil.c
# End Source File
# Begin Source File

SOURCE=..\..\..\sdk\zlib\zutil.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\source\controlsgui.cpp
# End Source File
# Begin Source File

SOURCE=.\drawtest.def
# End Source File
# Begin Source File

SOURCE=..\source\pdrawtesteditor.cpp
# End Source File
# Begin Source File

SOURCE=..\source\pdrawtesteffect.cpp
# End Source File
# Begin Source File

SOURCE=..\source\pdrawtestmain.cpp
# End Source File
# Begin Source File

SOURCE=..\source\pdrawtestview.cpp
# End Source File
# Begin Source File

SOURCE=..\source\pprimitivesviews.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\vstgui\aeffguieditor.h
# End Source File
# Begin Source File

SOURCE=..\..\vstgui\vstcontrols.h
# End Source File
# Begin Source File

SOURCE=..\..\vstgui\vstgui.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\drawtest.rc

!IF  "$(CFG)" == "drawtest - Win32 Release"

!ELSEIF  "$(CFG)" == "drawtest - Win32 Debug"

# ADD BASE RSC /l 0x809
# ADD RSC /l 0x809 /i "..\resources"

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
