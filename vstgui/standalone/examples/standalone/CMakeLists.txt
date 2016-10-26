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

##########################################################################################
if(CMAKE_HOST_APPLE)
	set(BUNDLE_PKG_INFO
    "resource/PkgInfo"
	)
	set(standalone_resources
		"resource/about.uidesc"
		"resource/alertbox.uidesc"
		"resource/resources.uidesc"
		"resource/test.uidesc"
		"resource/testpopup.uidesc"
	)
	set_source_files_properties(${BUNDLE_PKG_INFO} PROPERTIES
		MACOSX_PACKAGE_LOCATION "."
	)
  set_source_files_properties(${standalone_resources} PROPERTIES
    MACOSX_PACKAGE_LOCATION "Resources"
  )
  set(standalone_resources ${standalone_resources} ${BUNDLE_PKG_INFO})
  
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
		OUTPUT_NAME "standalone"
	)
endif()

##########################################################################################
if(MSVC)
  set(standalone_resources
    "resource/standalone.rc"
  )
  set(APP_PROPERTIES
    LINK_FLAGS "/SUBSYSTEM:windows /INCLUDE:wWinMain"
  )
endif()

##########################################################################################
include_directories(../../../../)
add_executable(${target}
  ${${target}_sources}
  ${standalone_resources}
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
