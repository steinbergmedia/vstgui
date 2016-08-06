##########################################################################################
set(vstgui_standalone_common_sources
  doc.h
  fwd.h
  ialertbox.h
  iappdelegate.h
  iapplication.h
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
set(vstgui_standalone_mac_sources
  detail/platform/mac/VSTGUICommand.h
  detail/platform/mac/VSTGUICommand.mm
  detail/platform/mac/macapplication.mm
  detail/platform/mac/macpreference.h
  detail/platform/mac/macpreference.mm
  detail/platform/mac/macutilities.h
  detail/platform/mac/macwindow.h
  detail/platform/mac/macwindow.mm
)

##########################################################################################
set(vstgui_standalone_win32_sources
  detail/platform/win32/win32application.cpp
  detail/platform/win32/win32preference.cpp
  detail/platform/win32/win32preference.h
  detail/platform/win32/win32window.cpp
  detail/platform/win32/win32window.h
)

##########################################################################################
if(CMAKE_HOST_APPLE)
  set(CMAKE_OSX_DEPLOYMENT_TARGET 10.9)
	set(vstgui_standalone_sources ${vstgui_standalone_common_sources} ${vstgui_standalone_mac_sources})

	set_source_files_properties(${vstgui_standalone_sources} PROPERTIES
		COMPILE_FLAGS "-fobjc-arc"
	)
endif()

##########################################################################################
if(MSVC)
	set(vstgui_standalone_sources ${vstgui_standalone_common_sources} ${vstgui_standalone_win32_sources})
endif()

##########################################################################################
add_library(vstgui_standalone STATIC ${vstgui_standalone_sources})

target_compile_definitions(vstgui_standalone ${VSTGUI_COMPILE_DEFINITIONS})

set_property(TARGET vstgui_standalone PROPERTY CXX_STANDARD 14)
set_property(TARGET vstgui_standalone PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET vstgui_standalone PROPERTY CMAKE_CXX_STANDARD_REQUIRED ON)

##########################################################################################
if(CMAKE_CONFIGURATION_TYPES)
  SOURCE_GROUP_BY_FOLDER(vstgui_standalone)
endif()
