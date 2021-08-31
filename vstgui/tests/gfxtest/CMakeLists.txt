##########################################################################################
# VSTGUI GFXTest Application
##########################################################################################
set(target GFXTest)

set(${target}_sources
  "source/app.cpp"
  "source/drawdevicetests.cpp"
  "source/drawdevicetests.h"
)

set(${target}_resources
  "resource/Window.uidesc"
  "resource/DrawDeviceTests.uidesc"
)

vstgui_add_executable(${target} "${${target}_sources}")
vstgui_add_resources(${target} "${${target}_resources}")
vstgui_set_target_bundle_id(${target} "vstgui.tests.gfxtest")
vstgui_set_target_infoplist(${target} "resource/Info.plist")
vstgui_set_target_rcfile(${target} "resource/gfxtest.rc")
vstgui_set_cxx_version(${target} 17)
target_include_directories(${target} PRIVATE ../../../)
set_target_properties(${target} PROPERTIES ${APP_PROPERTIES} FOLDER Tests)

