//-----------------------------------------------------------------------------
// VSTGUI: Graphical User Interface Framework not only for VST plugins
//
// Doxygen Documentation
//
// Version 4.3
//
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2011, Steinberg Media Technologies, All Rights Reserved
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

/**

@mainpage

Welcome to VSTGUI

- @ref intro @n
- @ref page_news_and_changes @n
- @ref page_vst3_inline_editing @n
- @ref page_vst2_tutorial @n
- @ref page_changelog @n
- @ref page_license @n

@section intro Introduction

\par VSTGUI 
VSTGUI is a User Interface Toolkit mainly for Audio Plug-Ins (VST, AudioUnit, etc).
\par History
First developed inhouse of Steinberg Media Technologies (around 1998) for their first VST Plug-Ins. 
Later added as binary libraries to the official VST SDK. 
Since May 2003 VSTGUI is open source and hosted at sourceforge.

Currently VSTGUI compiles on
\par Microsoft Windows (with Visual Studio and Windows 7 Platform SDK)
- XP SP 2 (32 and 64 bit)
- Vista (32 and 64 bit)
- 7 (32 and 64 bit)
- 8 (32 and 64 bit)
- 10 (32 and 64 bit)
\par Apple Mac OS X (with gcc 4.0/4.2 or Clang 2.0)
- 10.6 - 10.12 (32 and 64 bit)
\par Apple iOS
- 7.0 - 10.0 (32 and 64 bit)
\par Linux (with gcc >= 5.4 or clang >= 3.8)
- Ubuntu 2016.04 or newer

\sa
<a href="http://vstgui.sf.net" target=_blank> VSTGUI @ Sourceforge </a>

@page page_setup Setup

\par To include VSTGUI in your projects you only have to add

- vstgui_win32.cpp (for Windows)
- vstgui_linux.cpp (for Linux)
- vstgui_mac.mm (for macOS)
- vstgui_ios.mm (for iOS)

to your IDE project and add a search path to the parent of the root folder of vstgui.

\par On macOS, you need to link to the following Frameworks:

- Accelerate
- Carbon
- Cocoa
- OpenGL
- QuartzCore

\par On iOS, you need to link to the following Frameworks:

- UIKit
- CoreGraphics
- ImageIO
- CoreText
- GLKit
- Accelerate
- QuartzCore

\par On Linux you have to install gtkmm3 and their dependencies:

Ubuntu based distribution:
- libgtkmm-3.0-dev

@verbatim
sudo apt-get install libgtkmm-3.0-dev
@endverbatim

*/
