##########################################################################################
# VSTGUI Simple Standalone Application
##########################################################################################
set(target mandelbrot)

set(${target}_sources
  "standalone/examples/mandelbrot/source/mandelbrot.h"
  "standalone/examples/mandelbrot/source/mandelbrotapp.cpp"
  "standalone/examples/mandelbrot/source/mandelbrotview.cpp"
  "standalone/examples/mandelbrot/source/mandelbrotview.h"
  "standalone/examples/mandelbrot/source/mandelbrotwindow.cpp"
  "standalone/examples/mandelbrot/source/mandelbrotwindow.h"
)

##########################################################################################
if(CMAKE_HOST_APPLE)
	set(BUNDLE_PKG_INFO
    "standalone/examples/mandelbrot/resource/PkgInfo"
	)
	set(mandelbrot_resources
		"standalone/examples/mandelbrot/resource/Window.uidesc"
	)
	set_source_files_properties(${BUNDLE_PKG_INFO} PROPERTIES
		MACOSX_PACKAGE_LOCATION "."
	)
  set_source_files_properties(${mandelbrot_resources} PROPERTIES
    MACOSX_PACKAGE_LOCATION "Resources"
  )
  set(mandelbrot_resources ${mandelbrot_resources} ${BUNDLE_PKG_INFO})
  
	set(${target}_PLATFORM_LIBS
		"-framework Cocoa"
		"-framework OpenGL"
		"-framework QuartzCore"
		"-framework Accelerate"
	)
	get_filename_component(InfoPlistFile "standalone/examples/mandelbrot/resource/Info.plist" ABSOLUTE)
	set(APP_PROPERTIES
		MACOSX_BUNDLE TRUE
		MACOSX_BUNDLE_INFO_PLIST ${InfoPlistFile}
		OUTPUT_NAME "mandelbrot"
	)
endif()

##########################################################################################
if(MSVC)
  set(mandelbrot_resources
    "standalone/examples/mandelbrot/resource/mandelbrot.rc"
  )
  set(APP_PROPERTIES
    LINK_FLAGS "/SUBSYSTEM:windows /INCLUDE:wWinMain"
  )
endif()

##########################################################################################
include_directories(../)
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
set_target_properties(${target} PROPERTIES ${APP_PROPERTIES})
target_compile_definitions(${target} ${VSTGUI_COMPILE_DEFINITIONS})
