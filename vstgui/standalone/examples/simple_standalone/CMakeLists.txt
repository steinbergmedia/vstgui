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
if(MSVC)
  set(simple_standalone_resources
    "resource/simple_standalone.rc"
  )
  set(APP_PROPERTIES
    LINK_FLAGS "/SUBSYSTEM:windows /INCLUDE:wWinMain"
  )
endif()

##########################################################################################
vstgui_add_executable(${target} "${${target}_sources}" "${simple_standalone_resources}")
vstgui_set_target_infoplist(${target} "resource/Info.plist")
vstgui_set_cxx_version(${target} 14)
target_include_directories(${target} PRIVATE ../../../../)

