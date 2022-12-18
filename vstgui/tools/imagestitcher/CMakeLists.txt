set(TargetName ImageStitcher)

set(${TargetName}_sources
  Readme.md
  source/app.cpp
  source/document.cpp
  source/document.h
  source/documentcontroller.cpp
  source/documentcontroller.h
  source/imageframesview.cpp
  source/imageframesview.h
  source/startupcontroller.cpp
  source/startupcontroller.h
)

set(${TargetName}_RESOURCES
  "resource/Window.uidesc"
  "resource/StartupWindow.uidesc"
)

vstgui_add_executable(${TargetName} "${${TargetName}_sources}")
vstgui_add_resources(${TargetName} "${${TargetName}_RESOURCES}")
vstgui_set_target_bundle_id(${TargetName} "com.steinberg.vstgui.imagestitcher")
vstgui_set_target_infoplist(${TargetName} "resource/Info.plist")
vstgui_set_target_rcfile(${TargetName} "resource/imagestitcher.rc")
vstgui_set_cxx_version(${TargetName} 17)
target_include_directories(${TargetName} PRIVATE ../../../)
set_target_properties(${TargetName} PROPERTIES ${APP_PROPERTIES} ${VSTGUI_TOOLS_FOLDER})

