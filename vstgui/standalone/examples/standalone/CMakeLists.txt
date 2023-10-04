##########################################################################################
# VSTGUI Simple Standalone Application
##########################################################################################
set(target standalone)

set(standalone_sources
  "source/AlertBoxDesign.cpp"
  "source/AlertBoxDesign.h"
  "source/testabout.cpp"
  "source/testabout.h"
  "source/testappdelegate.cpp"
  "source/testappdelegate.h"
  "source/testmodel.cpp"
  "source/testmodel.h"
  "../../../contrib/datepicker.h"
  "../../../contrib/evbutton.h"
)

set(standalone_resources
  "resource/about.uidesc"
  "resource/alertbox.uidesc"
  "resource/resources.uidesc"
  "resource/test.uidesc"
  "resource/testpopup.uidesc"
  "resource/metalwindow.uidesc"
  "resource/direct3dwindow.uidesc"
)

set(standalone_font
  "resource/font/EffectsEighty.ttf"
)

set(standalone_scripts
  "resource/scripts/changecoloronfocus.js"
  "resource/scripts/hoveranim.js"
  "resource/scripts/test.js"
  "resource/scripts/viewcontainertest.js"
)

#########################################################################################
if(CMAKE_HOST_APPLE)
  set(standalone_sources
    ${standalone_sources}
    "source/metalwindow.mm"
    "source/metalwindow.h"
    "source/metaltypes.h"
    "source/metalshader.h"
    "../../../contrib/externalview_nsview.h"
    "../../../contrib/externalview_metal.h"
    "../../../contrib/datepicker.mm"
    "../../../contrib/evbutton_macos.mm"
  )
elseif(MSVC)
  set(standalone_sources
    ${standalone_sources}
    "../../../contrib/externalview_hwnd.h"
    "../../../contrib/externalview_direct3d12.h"
    "../../../contrib/datepicker_win32.cpp"
    "../../../contrib/evbutton_win32.cpp"
	"source/direct3dwindow.cpp"
	"source/direct3dwindow.h"
	"source/direct3dshader.h"
  )
endif()

##########################################################################################
vstgui_add_executable(${target} "${standalone_sources}")
vstgui_add_resources(${target} "${standalone_resources}")
vstgui_add_resources(${target} "${standalone_font}" "Fonts")
vstgui_add_resources(${target} "${standalone_scripts}")
vstgui_set_target_bundle_id(${target} "vstgui.examples.standalone")
vstgui_set_target_infoplist(${target} "resource/Info.plist")
vstgui_set_target_rcfile(${target} "resource/standalone.rc")
vstgui_set_cxx_version(${target} 17)
if(VSTGUI_UISCRIPTING)
    target_link_libraries(${target} PRIVATE vstgui_uiscripting)
endif(VSTGUI_UISCRIPTING)
target_include_directories(${target} PRIVATE ../../../../)
set_target_properties(${target} PROPERTIES ${APP_PROPERTIES} ${VSTGUI_STANDALONE_EXAMPLES_FOLDER})

if(CMAKE_HOST_APPLE)
	target_link_libraries(${target} PRIVATE
		"-framework Metal"
	)
endif()

source_group(source FILES ${standalone_sources})
source_group(resource FILES ${standalone_resources} "resource/standalone.rc")
source_group(resource/scripts FILES ${standalone_scripts})
source_group(resource/font FILES ${standalone_font})
