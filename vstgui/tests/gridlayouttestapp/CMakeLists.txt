##########################################################################################
# VSTGUI Grid Layout Test App
##########################################################################################
option(VSTGUI_GRIDLAYOUTTESTAPP "Grid Layout Test App" OFF)

if(NOT VSTGUI_GRIDLAYOUTTESTAPP)
	return()
endif()

set(target gridlayouttestapp)

set(${target}_sources
  "source/app.cpp"
)

set(${target}_resources
"resource/Window.uidesc"
)

##########################################################################################
vstgui_add_executable(${target} "${${target}_sources}" )
vstgui_add_resources(${target} "${${target}_resources}")
vstgui_set_target_bundle_id(${target} "vstgui.examples.gridlayouttestapp")
vstgui_set_target_infoplist(${target} "resource/Info.plist")
vstgui_set_target_rcfile(${target} "resource/windows.rc")
vstgui_set_cxx_version(${target} 17)
target_include_directories(${target} PRIVATE ../../../)
set_target_properties(${target} PROPERTIES ${APP_PROPERTIES} FOLDER Tests)

