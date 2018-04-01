set(TargetName ImageStitcher)

set(${TargetName}_sources
  source/app.cpp
  source/document.cpp
  source/document.h
  source/documentcontroller.cpp
  source/documentcontroller.h
  source/imageframesview.cpp
  source/imageframesview.h
)

if(CMAKE_HOST_APPLE)

  set(BUNDLE_PKG_INFO
    "resource/PkgInfo"
  )
  set_source_files_properties(${BUNDLE_PKG_INFO} PROPERTIES
    MACOSX_PACKAGE_LOCATION "."
  )

  set(${TargetName}_PLATFORM_LIBS
    "-framework Cocoa"
    "-framework OpenGL"
    "-framework QuartzCore"
    "-framework Accelerate"
    "-framework CoreAudio"
  )
  set(${TargetName}_RESOURCES
    "resource/Window.uidesc"
  )
  set_source_files_properties(${${TargetName}_RESOURCES} PROPERTIES
    MACOSX_PACKAGE_LOCATION Resources
  )

  set(${TargetName}_RESOURCES ${${TargetName}_RESOURCES} ${BUNDLE_PKG_INFO})

  get_filename_component(InfoPlistFile "resource/Info.plist" ABSOLUTE)
  set(APP_PROPERTIES
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_INFO_PLIST ${InfoPlistFile}
    OUTPUT_NAME ${TargetName}
    XCODE_ATTRIBUTE_USER_HEADER_SEARCH_PATHS ../
  )
endif()
if(MSVC)
  SET(${TargetName}_SOURCE_FILES
    ${${TargetName}_SOURCE_FILES}
    ${WIN32_PLUG_EXPORT_DEF}
    "resource/imagestitcher.rc"
  )
  set(APP_PROPERTIES
    LINK_FLAGS "/SUBSYSTEM:windows /INCLUDE:wWinMain"
  )
  set(${TargetName}_RESOURCES ${${TargetName}_RESOURCES}
    "resource/imagestitcher.rc"
  )
endif()

add_executable(${TargetName}
  ${${TargetName}_sources}
  ${${TargetName}_RESOURCES}
)
target_link_libraries(${TargetName}
  vstgui
  vstgui_uidescription
  vstgui_standalone
  ${${TargetName}_PLATFORM_LIBS}
)
target_include_directories(${TargetName} PRIVATE ../../../)

vstgui_set_cxx_version(${TargetName} 14)
set_target_properties(${TargetName} PROPERTIES ${APP_PROPERTIES} ${VSTGUI_TOOLS})
target_compile_definitions(${TargetName} ${VSTGUI_COMPILE_DEFINITIONS})
# vstgui_source_group_by_folder(${TargetName})
