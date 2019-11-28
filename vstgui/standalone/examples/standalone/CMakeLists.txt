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
)

set(standalone_resources
  "resource/about.uidesc"
  "resource/alertbox.uidesc"
  "resource/resources.uidesc"
  "resource/test.uidesc"
  "resource/testpopup.uidesc"
)

set(standalone_font
  "resource/font/EffectsEighty.ttf"
)

##########################################################################################
vstgui_add_executable(${target} "${standalone_sources}")
vstgui_add_resources(${target} "${standalone_resources}")
vstgui_add_resources(${target} "${standalone_font}" "Fonts")
vstgui_set_target_infoplist(${target} "resource/Info.plist")
vstgui_set_target_rcfile(${target} "resource/standalone.rc")
vstgui_set_cxx_version(${target} 14)
target_include_directories(${target} PRIVATE ../../../../)
set_target_properties(${target} PROPERTIES ${APP_PROPERTIES} ${VSTGUI_STANDALONE_EXAMPLES_FOLDER})
