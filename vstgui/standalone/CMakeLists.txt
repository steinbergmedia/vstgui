##########################################################################################
set(target vstgui_standalone)

set(${target}_common_sources
  include/doc.h
  include/fwd.h
  include/ialertbox.h
  include/iappdelegate.h
  include/iapplication.h
  include/iasync.h
  include/icommand.h
  include/icommondirectories.h
  include/imenubuilder.h
  include/interface.h
  include/ipreference.h
  include/ishareduiresources.h
  include/iuidescwindow.h
  include/ivalue.h
  include/ivaluelistener.h
  include/iwindow.h
  include/iwindowcontroller.h
  include/iwindowlistener.h
  include/helpers/appdelegate.h
  include/helpers/menubuilder.h
  include/helpers/preferences.h
  include/helpers/value.h
  include/helpers/valuelistener.h
  include/helpers/windowcontroller.h
  include/helpers/windowlistener.h
  include/helpers/uidesc/modelbinding.h
  include/helpers/uidesc/customization.h
  source/application.cpp
  source/application.h
  source/genericalertbox.cpp
  source/genericalertbox.h
  source/shareduiresources.cpp
  source/shareduiresources.h
  source/uidescriptionwindowcontroller.cpp
  source/window.cpp
  source/window.h
  source/platform/iplatformwindow.h
  source/helpers/value.cpp
)

##########################################################################################
set(${target}_mac_sources
  source/platform/mac/VSTGUICommand.h
  source/platform/mac/VSTGUICommand.mm
  source/platform/mac/macasync.mm
  source/platform/mac/macasync.h
  source/platform/mac/macapplication.mm
  source/platform/mac/maccommondirectories.h
  source/platform/mac/maccommondirectories.mm
  source/platform/mac/macpreference.h
  source/platform/mac/macpreference.mm
  source/platform/mac/macutilities.h
  source/platform/mac/macwindow.h
  source/platform/mac/macwindow.mm
)

##########################################################################################
set(${target}_win32_sources
  source/platform/win32/win32application.cpp
  source/platform/win32/win32async.cpp
  source/platform/win32/win32async.h
  source/platform/win32/win32commondirectories.h
  source/platform/win32/win32commondirectories.cpp
  source/platform/win32/win32menu.cpp
  source/platform/win32/win32menu.h
  source/platform/win32/win32preference.cpp
  source/platform/win32/win32preference.h
  source/platform/win32/win32window.cpp
  source/platform/win32/win32window.h
)

##########################################################################################
if(CMAKE_HOST_APPLE)
	set(${target}_sources ${${target}_common_sources} ${${target}_mac_sources})

	set_source_files_properties(${${target}_sources} PROPERTIES
		COMPILE_FLAGS "-fobjc-arc"
	)
endif()

##########################################################################################
if(MSVC)
	set(${target}_sources ${${target}_common_sources} ${${target}_win32_sources})
endif()

##########################################################################################
add_library(${target} STATIC ${${target}_sources})

target_compile_definitions(${target} ${VSTGUI_COMPILE_DEFINITIONS})
vstgui_set_cxx_version(${target} 14)
vstgui_source_group_by_folder(${target})

if(NOT VSTGUI_DISABLE_STANDALONE_EXAMPLES)
  add_subdirectory(examples)
endif()
