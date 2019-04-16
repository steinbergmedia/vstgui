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
)

set(mandelbrot_resources
  "resource/Window.uidesc"
)

##########################################################################################
if(CMAKE_HOST_APPLE)
	set(${target}_sources ${${target}_sources}
		"source/touchbarsupport.mm"
		"source/touchbarsupport.h"
	)
endif()

##########################################################################################
vstgui_add_executable(${target} "${${target}_sources}")
vstgui_add_resources(${target} "${mandelbrot_resources}")
vstgui_set_target_infoplist(${target} "resource/Info.plist")
vstgui_set_target_rcfile(${target} "resource/mandelbrot.rc")
vstgui_set_cxx_version(${target} 14)
target_include_directories(${target} PRIVATE ../../../../)
set_target_properties(${target} PROPERTIES ${APP_PROPERTIES} ${VSTGUI_STANDALONE_EXAMPLES_FOLDER})
