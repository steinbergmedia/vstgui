##########################################################################################
# VSTGUI Simple Standalone Application
##########################################################################################
set(target simple_standalone)

set(${target}_sources
  "source/myapp.cpp"
)

##########################################################################################
if(CMAKE_HOST_APPLE)
	set(BUNDLE_PKG_INFO
    "resource/PkgInfo"
	)
	set(simple_standalone_resources
		"resource/Window.uidesc"
	)
	set_source_files_properties(${BUNDLE_PKG_INFO} PROPERTIES
		MACOSX_PACKAGE_LOCATION "."
	)
  set_source_files_properties(${simple_standalone_resources} PROPERTIES
    MACOSX_PACKAGE_LOCATION "Resources"
  )
  set(simple_standalone_resources ${simple_standalone_resources} ${BUNDLE_PKG_INFO})
  
	set(${target}_PLATFORM_LIBS
		"-framework Cocoa"
		"-framework OpenGL"
		"-framework QuartzCore"
		"-framework Accelerate"
	)
	get_filename_component(InfoPlistFile "resource/Info.plist" ABSOLUTE)
	set(APP_PROPERTIES
		MACOSX_BUNDLE TRUE
		MACOSX_BUNDLE_INFO_PLIST ${InfoPlistFile}
		OUTPUT_NAME "simple_standalone"
	)
endif()

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
include_directories(../../../../)
add_executable(${target}
  ${${target}_sources}
  ${simple_standalone_resources}
)
target_link_libraries(${target}
	vstgui
  vstgui_uidescription
  vstgui_standalone
	${${target}_PLATFORM_LIBS}
)

vstgui_set_cxx_version(${target} 14)
set_target_properties(${target} PROPERTIES ${APP_PROPERTIES} ${VSTGUI_STANDALONE_EXAMPLES})
target_compile_definitions(${target} ${VSTGUI_COMPILE_DEFINITIONS})
