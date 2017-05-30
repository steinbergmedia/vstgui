// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

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
