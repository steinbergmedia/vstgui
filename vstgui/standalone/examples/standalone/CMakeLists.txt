##########################################################################################
# VSTGUI Simple Standalone Application
##########################################################################################
set(target standalone)

set(${target}_sources
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

##########################################################################################
vstgui_add_executable(${target} "${${target}_sources}" "${standalone_resources}")
vstgui_set_target_infoplist(${target} "resource/Info.plist")
vstgui_set_target_rcfile(${target} "resource/standalone.rc")
vstgui_set_cxx_version(${target} 14)
target_include_directories(${target} PRIVATE ../../../../)
set_target_properties(${target} PROPERTIES ${APP_PROPERTIES} ${VSTGUI_STANDALONE_EXAMPLES_FOLDER})

