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
if(MSVC)
  set(mandelbrot_resources
    "resource/mandelbrot.rc"
  )
  set(APP_PROPERTIES
    LINK_FLAGS "/SUBSYSTEM:windows /INCLUDE:wWinMain"
  )
endif()

##########################################################################################
vstgui_add_executable(${target} "${${target}_sources}" "${mandelbrot_resources}")
vstgui_set_target_infoplist(${target} "resource/Info.plist")
vstgui_set_cxx_version(${target} 14)
target_include_directories(${target} PRIVATE ../../../../)
