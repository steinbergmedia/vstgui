##########################################################################################
# VSTGUI Simple Standalone Application
##########################################################################################
set(target simple_standalone)

set(${target}_sources
  "source/myapp.cpp"
)

set(simple_standalone_resources
"resource/Window.uidesc"
)

##########################################################################################
vstgui_add_executable(${target} "${${target}_sources}" )
vstgui_add_resources(${target} "${simple_standalone_resources}")
vstgui_set_target_infoplist(${target} "resource/Info.plist")
vstgui_set_target_rcfile(${target} "resource/simple_standalone.rc")
vstgui_set_cxx_version(${target} 14)
target_include_directories(${target} PRIVATE ../../../../)
set_target_properties(${target} PROPERTIES ${APP_PROPERTIES} ${VSTGUI_STANDALONE_EXAMPLES_FOLDER})

