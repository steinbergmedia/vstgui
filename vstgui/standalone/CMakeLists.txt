##########################################################################################
set(target vstgui_standalone)

set(${target}_common_sources
  doc.h
  fwd.h
  ialertbox.h
  iappdelegate.h
  iapplication.h
  iasync.h
  icommand.h
  interface.h
  ipreference.h
  ishareduiresources.h
  iuidescwindow.h
  ivalue.h
  ivaluelistener.h
  iwindow.h
  iwindowcontroller.h
  iwindowlistener.h
  detail/application.cpp
  detail/application.h
  detail/genericalertbox.cpp
  detail/genericalertbox.h
  detail/shareduiresources.cpp
  detail/shareduiresources.h
  detail/uidescriptionwindowcontroller.cpp
  detail/window.cpp
  detail/window.h
  detail/platform/iplatformwindow.h
  helpers/appdelegate.h
  helpers/modelbinding.h
  helpers/value.cpp
  helpers/value.h
  helpers/valuelistener.h
  helpers/windowcontroller.h
  helpers/windowlistener.h
)

##########################################################################################
set(${target}_mac_sources
  detail/platform/mac/VSTGUICommand.h
  detail/platform/mac/VSTGUICommand.mm
  detail/platform/mac/macasync.mm
  detail/platform/mac/macapplication.mm
  detail/platform/mac/macpreference.h
  detail/platform/mac/macpreference.mm
  detail/platform/mac/macutilities.h
  detail/platform/mac/macwindow.h
  detail/platform/mac/macwindow.mm
)

##########################################################################################
set(${target}_win32_sources
  detail/platform/win32/win32application.cpp
  detail/platform/win32/win32async.cpp
  detail/platform/win32/win32async.h
  detail/platform/win32/win32menu.cpp
  detail/platform/win32/win32menu.h
  detail/platform/win32/win32preference.cpp
  detail/platform/win32/win32preference.h
  detail/platform/win32/win32window.cpp
  detail/platform/win32/win32window.h
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
