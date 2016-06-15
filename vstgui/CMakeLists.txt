cmake_minimum_required(VERSION 3.4.1)
project(vstgui)

if(CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_CONFIGURATION_TYPES Debug Release)
  set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif()

##########################################################################################
set(vstgui_common_sources 
	lib/cbitmap.cpp
	lib/cbitmap.h
	lib/cbitmapfilter.cpp
	lib/cbitmapfilter.h
	lib/cbuttonstate.h
	lib/ccolor.cpp
	lib/ccolor.h
	lib/cdatabrowser.cpp
	lib/cdatabrowser.h
	lib/cdrawcontext.cpp
	lib/cdrawcontext.h
	lib/cdrawdefs.h
	lib/cdrawmethods.cpp
	lib/cdrawmethods.h
	lib/cdropsource.cpp
	lib/cdropsource.h
	lib/cfileselector.cpp
	lib/cfileselector.h
	lib/cfont.cpp
	lib/cfont.h
	lib/cframe.cpp
	lib/cframe.h
	lib/cgradient.h
	lib/cgradientview.cpp
	lib/cgradientview.h
	lib/cgraphicspath.cpp
	lib/cgraphicspath.h
	lib/cgraphicstransform.h
	lib/clayeredviewcontainer.cpp
	lib/clayeredviewcontainer.h
	lib/clinestyle.cpp
	lib/clinestyle.h
	lib/coffscreencontext.cpp
	lib/coffscreencontext.h
	lib/copenglview.cpp
	lib/copenglview.h
	lib/cpoint.cpp
	lib/cpoint.h
	lib/crect.cpp
	lib/crect.h
	lib/cresourcedescription.h
	lib/crowcolumnview.cpp
	lib/crowcolumnview.h
	lib/cscrollview.cpp
	lib/cscrollview.h
	lib/cshadowviewcontainer.cpp
	lib/cshadowviewcontainer.h
	lib/csplitview.cpp
	lib/csplitview.h
	lib/cstring.cpp
	lib/cstring.h
	lib/ctabview.cpp
	lib/ctabview.h
	lib/ctooltipsupport.cpp
	lib/ctooltipsupport.h
	lib/cview.cpp
	lib/cview.h
	lib/cviewcontainer.cpp
	lib/cviewcontainer.h
	lib/cvstguitimer.cpp
	lib/cvstguitimer.h
	lib/dispatchlist.h
	lib/idatapackage.h
	lib/idependency.h
	lib/ifocusdrawing.h
	lib/iscalefactorchangedlistener.h
	lib/itouchevent.h
	lib/iviewlistener.h
	lib/vstguibase.h
	lib/vstguidebug.cpp
	lib/vstguidebug.h
	lib/vstguifwd.h
	lib/vstkeycode.h
	lib/animation/animations.cpp
	lib/animation/animations.h
	lib/animation/animator.cpp
	lib/animation/animator.h
	lib/animation/ianimationtarget.h
	lib/animation/itimingfunction.h
	lib/animation/timingfunctions.cpp
	lib/animation/timingfunctions.h
	lib/controls/cautoanimation.cpp
	lib/controls/cautoanimation.h
	lib/controls/cbuttons.cpp
	lib/controls/cbuttons.h
	lib/controls/ccolorchooser.cpp
	lib/controls/ccolorchooser.h
	lib/controls/ccontrol.cpp
	lib/controls/ccontrol.h
	lib/controls/cfontchooser.cpp
	lib/controls/cfontchooser.h
	lib/controls/cknob.cpp
	lib/controls/cknob.h
	lib/controls/cmoviebitmap.cpp
	lib/controls/cmoviebitmap.h
	lib/controls/cmoviebutton.cpp
	lib/controls/cmoviebutton.h
	lib/controls/coptionmenu.cpp
	lib/controls/coptionmenu.h
	lib/controls/cparamdisplay.cpp
	lib/controls/cparamdisplay.h
	lib/controls/cscrollbar.cpp
	lib/controls/cscrollbar.h
	lib/controls/csegmentbutton.cpp
	lib/controls/csegmentbutton.h
	lib/controls/cslider.cpp
	lib/controls/cslider.h
	lib/controls/cspecialdigit.cpp
	lib/controls/cspecialdigit.h
	lib/controls/csplashscreen.cpp
	lib/controls/csplashscreen.h
	lib/controls/cswitch.cpp
	lib/controls/cswitch.h
	lib/controls/ctextedit.cpp
	lib/controls/ctextedit.h
	lib/controls/ctextlabel.cpp
	lib/controls/ctextlabel.h
	lib/controls/cvumeter.cpp
	lib/controls/cvumeter.h
	lib/controls/cxypad.cpp
	lib/controls/cxypad.h
	lib/controls/icontrollistener.h
	lib/platform/iplatformbitmap.h
	lib/platform/iplatformfont.h
	lib/platform/iplatformframe.h
	lib/platform/iplatformopenglview.h
	lib/platform/iplatformoptionmenu.h
	lib/platform/iplatformstring.h
	lib/platform/iplatformtextedit.h
	lib/platform/iplatformtimer.h
	lib/platform/iplatformviewlayer.h
	lib/platform/std_unorderedmap.h
)

##########################################################################################
set(vstgui_uidescription_common_sources
	uidescription/base64codec.h
	uidescription/cstream.cpp
	uidescription/cstream.h
	uidescription/delegationcontroller.h
	uidescription/doc.h
	uidescription/icontroller.h
	uidescription/iuidescription.h
	uidescription/iviewcreator.h
	uidescription/iviewfactory.h
	uidescription/uiattributes.cpp
	uidescription/uiattributes.h
	uidescription/uidescription.cpp
	uidescription/uidescription.h
	uidescription/uidescriptionfwd.h
	uidescription/uiviewcreator.cpp
	uidescription/uiviewcreator.h
	uidescription/uiviewfactory.cpp
	uidescription/uiviewfactory.h
	uidescription/uiviewswitchcontainer.cpp
	uidescription/uiviewswitchcontainer.h
	uidescription/xmlparser.cpp
	uidescription/xmlparser.h
	uidescription/editing/doc.h
	uidescription/editing/iaction.h
	uidescription/editing/uiactions.cpp
	uidescription/editing/uiactions.h
	uidescription/editing/uiattributescontroller.cpp
	uidescription/editing/uiattributescontroller.h
	uidescription/editing/uibasedatasource.h
	uidescription/editing/uibitmapscontroller.cpp
	uidescription/editing/uibitmapscontroller.h
	uidescription/editing/uicolor.cpp
	uidescription/editing/uicolor.h
	uidescription/editing/uicolorchoosercontroller.cpp
	uidescription/editing/uicolorchoosercontroller.h
	uidescription/editing/uicolorscontroller.cpp
	uidescription/editing/uicolorscontroller.h
	uidescription/editing/uicolorslider.cpp
	uidescription/editing/uicolorslider.h
	uidescription/editing/uicrosslines.cpp
	uidescription/editing/uicrosslines.h
	uidescription/editing/uidescriptioneditor.uidesc
	uidescription/editing/uidialogcontroller.cpp
	uidescription/editing/uidialogcontroller.h
	uidescription/editing/uieditcontroller.cpp
	uidescription/editing/uieditcontroller.h
	uidescription/editing/uieditmenucontroller.cpp
	uidescription/editing/uieditmenucontroller.h
	uidescription/editing/uieditview.cpp
	uidescription/editing/uieditview.h
	uidescription/editing/uifocussettingscontroller.cpp
	uidescription/editing/uifocussettingscontroller.h
	uidescription/editing/uifontscontroller.cpp
	uidescription/editing/uifontscontroller.h
	uidescription/editing/uigradientscontroller.cpp
	uidescription/editing/uigradientscontroller.h
	uidescription/editing/uigrid.h
	uidescription/editing/uigridcontroller.cpp
	uidescription/editing/uigridcontroller.h
	uidescription/editing/uisearchtextfield.cpp
	uidescription/editing/uisearchtextfield.h
	uidescription/editing/uiselection.cpp
	uidescription/editing/uiselection.h
	uidescription/editing/uitagscontroller.cpp
	uidescription/editing/uitagscontroller.h
	uidescription/editing/uitemplatecontroller.cpp
	uidescription/editing/uitemplatecontroller.h
	uidescription/editing/uitemplatesettingscontroller.cpp
	uidescription/editing/uitemplatesettingscontroller.h
	uidescription/editing/uiundomanager.cpp
	uidescription/editing/uiundomanager.h
	uidescription/editing/uiviewcreatecontroller.cpp
	uidescription/editing/uiviewcreatecontroller.h
)

##########################################################################################
set(vstgui_mac_sources
	lib/platform/mac/caviewlayer.h
	lib/platform/mac/caviewlayer.mm
	lib/platform/mac/cfontmac.h
	lib/platform/mac/cfontmac.mm
	lib/platform/mac/cgbitmap.cpp
	lib/platform/mac/cgbitmap.h
	lib/platform/mac/cgdrawcontext.cpp
	lib/platform/mac/cgdrawcontext.h
	lib/platform/mac/macclipboard.h
	lib/platform/mac/macclipboard.mm
	lib/platform/mac/macfileselector.mm
	lib/platform/mac/macglobals.cpp
	lib/platform/mac/macglobals.h
	lib/platform/mac/macstring.h
	lib/platform/mac/macstring.mm
	lib/platform/mac/mactimer.cpp
	lib/platform/mac/quartzgraphicspath.cpp
	lib/platform/mac/quartzgraphicspath.h
	lib/platform/mac/cocoa/autoreleasepool.h
	lib/platform/mac/cocoa/autoreleasepool.mm
	lib/platform/mac/cocoa/cocoahelpers.h
	lib/platform/mac/cocoa/cocoahelpers.mm
	lib/platform/mac/cocoa/cocoaopenglview.h
	lib/platform/mac/cocoa/cocoaopenglview.mm
	lib/platform/mac/cocoa/cocoatextedit.h
	lib/platform/mac/cocoa/cocoatextedit.mm
	lib/platform/mac/cocoa/nsviewframe.h
	lib/platform/mac/cocoa/nsviewframe.mm
	lib/platform/mac/cocoa/nsviewoptionmenu.h
	lib/platform/mac/cocoa/nsviewoptionmenu.mm
	lib/platform/mac/carbon/hiviewframe.cpp
	lib/platform/mac/carbon/hiviewframe.h
	lib/platform/mac/carbon/hiviewoptionmenu.cpp
	lib/platform/mac/carbon/hiviewoptionmenu.h
	lib/platform/mac/carbon/hiviewtextedit.cpp
	lib/platform/mac/carbon/hiviewtextedit.h
	lib/platform/win32
)

##########################################################################################
set(vstgui_win32_sources
	lib/platform/win32/cfontwin32.cpp
	lib/platform/win32/cfontwin32.h
	lib/platform/win32/gdiplusbitmap.cpp
	lib/platform/win32/gdiplusbitmap.h
	lib/platform/win32/gdiplusdrawcontext.cpp
	lib/platform/win32/gdiplusdrawcontext.h
	lib/platform/win32/gdiplusgraphicspath.cpp
	lib/platform/win32/gdiplusgraphicspath.h
	lib/platform/win32/win32bitmapbase.h
	lib/platform/win32/win32dragcontainer.cpp
	lib/platform/win32/win32dragcontainer.h
	lib/platform/win32/win32frame.cpp
	lib/platform/win32/win32frame.h
	lib/platform/win32/win32openglview.cpp
	lib/platform/win32/win32openglview.h
	lib/platform/win32/win32optionmenu.cpp
	lib/platform/win32/win32optionmenu.h
	lib/platform/win32/win32support.cpp
	lib/platform/win32/win32support.h
	lib/platform/win32/win32textedit.cpp
	lib/platform/win32/win32textedit.h
	lib/platform/win32/winfileselector.cpp
	lib/platform/win32/winstring.cpp
	lib/platform/win32/winstring.h
	lib/platform/win32/wintimer.cpp
	lib/platform/win32/direct2d/d2dbitmap.cpp
	lib/platform/win32/direct2d/d2dbitmap.h
	lib/platform/win32/direct2d/d2ddrawcontext.cpp
	lib/platform/win32/direct2d/d2ddrawcontext.h
	lib/platform/win32/direct2d/d2dfont.cpp
	lib/platform/win32/direct2d/d2dfont.h
	lib/platform/win32/direct2d/d2dgraphicspath.cpp
	lib/platform/win32/direct2d/d2dgraphicspath.h
)

##########################################################################################
set(vstgui_standalone_common_sources
	standalone/fwd.h
	standalone/ialertbox.h
	standalone/iappdelegate.h
	standalone/iapplication.h
	standalone/icommand.h
	standalone/interface.h
	standalone/ipreference.h
	standalone/iuidescwindow.h
	standalone/ivalue.h
	standalone/ivaluelistener.h
	standalone/iwindow.h
	standalone/iwindowcontroller.h
	standalone/iwindowlistener.h
	standalone/detail/application.cpp
	standalone/detail/application.h
	standalone/detail/genericalertbox.cpp
	standalone/detail/genericalertbox.h
	standalone/detail/uidescriptionwindowcontroller.cpp
	standalone/detail/window.cpp
	standalone/detail/window.h
	standalone/detail/platform/iplatformwindow.h
)

##########################################################################################
set(vstgui_standalone_mac_sources
	standalone/detail/platform/mac/VSTGUICommand.h
	standalone/detail/platform/mac/VSTGUICommand.mm
	standalone/detail/platform/mac/macapplication.mm
	standalone/detail/platform/mac/macpreference.h
	standalone/detail/platform/mac/macpreference.mm
	standalone/detail/platform/mac/macutilities.h
	standalone/detail/platform/mac/macwindow.h
	standalone/detail/platform/mac/macwindow.mm
)

##########################################################################################
set(vstgui_standalone_win32_sources
	standalone/detail/platform/win32/win32application.cpp
	standalone/detail/platform/win32/win32preference.cpp
	standalone/detail/platform/win32/win32preference.h
	standalone/detail/platform/win32/win32window.cpp
	standalone/detail/platform/win32/win32window.h
)

##########################################################################################
if(CMAKE_HOST_APPLE)
  set(CMAKE_OSX_DEPLOYMENT_TARGET 10.9)
	set(vstgui_sources ${vstgui_common_sources} ${vstgui_uidescription_common_sources} ${vstgui_mac_sources})
	set(vstgui_standalone_sources ${vstgui_standalone_common_sources} ${vstgui_standalone_mac_sources})

	set_source_files_properties(${vstgui_standalone_sources} PROPERTIES
		COMPILE_FLAGS "-fobjc-arc"
	)
endif()

##########################################################################################
if(MSVC)
	set(vstgui_sources ${vstgui_common_sources} ${vstgui_uidescription_common_sources} ${vstgui_win32_sources})
	set(vstgui_standalone_sources ${vstgui_standalone_common_sources} ${vstgui_standalone_win32_sources})
endif()

##########################################################################################
set(VSTGUI_COMPILE_DEFINITIONS_DEBUG "VSTGUI_LIVE_EDITING;DEBUG")
set(VSTGUI_COMPILE_DEFINITIONS_RELEASE "NDEBUG;RELEASE")
if(MSVC)
  set(VSTGUI_COMPILE_DEFINITIONS_DEBUG "${VSTGUI_COMPILE_DEFINITIONS_DEBUG};_CRT_SECURE_NO_WARNINGS")
  set(VSTGUI_COMPILE_DEFINITIONS_RELEASE "${VSTGUI_COMPILE_DEFINITIONS_RELEASE};_CRT_SECURE_NO_WARNINGS")
endif()

set(VSTGUI_COMPILE_DEFINITIONS PRIVATE
    $<$<CONFIG:Debug>:${VSTGUI_COMPILE_DEFINITIONS_DEBUG}>
    $<$<CONFIG:Release>:${VSTGUI_COMPILE_DEFINITIONS_RELEASE}>
)

add_library(vstgui STATIC ${vstgui_sources})
add_library(vstgui_standalone STATIC ${vstgui_standalone_sources})

target_compile_definitions(vstgui ${VSTGUI_COMPILE_DEFINITIONS})
target_compile_definitions(vstgui_standalone ${VSTGUI_COMPILE_DEFINITIONS})

set_property(TARGET vstgui PROPERTY CXX_STANDARD 11)
set_property(TARGET vstgui PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET vstgui PROPERTY CMAKE_CXX_STANDARD_REQUIRED ON)
set_property(TARGET vstgui_standalone PROPERTY CXX_STANDARD 14)
set_property(TARGET vstgui_standalone PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET vstgui_standalone PROPERTY CMAKE_CXX_STANDARD_REQUIRED ON)

##########################################################################################
macro(SOURCE_GROUP_BY_FOLDER target)
  set(SOURCE_GROUP_DELIMITER "/")
  set(last_dir "")
  set(files "")
  foreach(file ${${target}_sources})
    get_filename_component(dir "${file}" DIRECTORY)
    string(FIND "${dir}" ${target} isTargetFolder)
    if (${isTargetFolder} EQUAL 0)
      string(LENGTH ${target} offset)
      string(SUBSTRING "${dir}" ${offset} -1 dir)
    endif(${isTargetFolder} EQUAL 0)
    if (NOT "${dir}" STREQUAL "${last_dir}")
      if (files)
        source_group("${last_dir}" FILES ${files})
      endif (files)
      set(files "")
    endif (NOT "${dir}" STREQUAL "${last_dir}")
    set(files ${files} ${file})
    set(last_dir "${dir}")
  endforeach(file)
  if (files)
    source_group("${last_dir}" FILES ${files})
  endif (files)
endmacro(SOURCE_GROUP_BY_FOLDER)

##########################################################################################
if(CMAKE_CONFIGURATION_TYPES)
  SOURCE_GROUP_BY_FOLDER(vstgui)
  SOURCE_GROUP_BY_FOLDER(vstgui_standalone)
endif()
