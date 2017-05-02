##########################################################################################
# VSTGUI Simple Standalone Application
##########################################################################################
set(target mandelbrot)

set(${target}_sources
  "source/mandelbrot.h"
  "source/mandelbrotapp.cpp"
  "source/mandelbrotview.cpp"
  "source/mandelbrotview.h"
  "source/mandelbrotwindow.cpp"
  "source/mandelbrotwindow.h"
  "source/modelbinding.h"
  "source/progressindicatorview.cpp"
)

##########################################################################################
if(CMAKE_HOST_APPLE)
	set(BUNDLE_PKG_INFO
    "resource/PkgInfo"
	)
	set(mandelbrot_resources
		"resource/Window.uidesc"
	)
	set_source_files_properties(${BUNDLE_PKG_INFO} PROPERTIES
		MACOSX_PACKAGE_LOCATION "."
	)
  set_source_files_properties(${mandelbrot_resources} PROPERTIES
    MACOSX_PACKAGE_LOCATION "Resources"
  )
  set(mandelbrot_resources ${mandelbrot_resources} ${BUNDLE_PKG_INFO})
  
	set(${target}_sources ${${target}_sources}
		"source/touchbarsupport.mm"
		"source/touchbarsupport.h"
	)

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
		OUTPUT_NAME "mandelbrot"
	)
endif()

##########################################################################################
if(MSVC)
  set(mandelbrot_resources
    "resource/mandelbrot.rc"
  )
  set(APP_PROPERTIES
    LINK_FLAGS "/SUBSYSTEM:windows /INCLUDE:wWinMain"
  )
endif()

##########################################################################################
include_directories(../../../../)
add_executable(${target}
  ${${target}_sources}
  ${mandelbrot_resources}
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
