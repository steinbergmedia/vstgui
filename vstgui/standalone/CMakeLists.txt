list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

include(vstgui_add_executable)

##########################################################################################
set(target vstgui_standalone)

set(${target}_common_sources
    include/appinit.h
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
    include/helpers/async.h
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
    source/platform/win32
    source/platform/gdk
    cmake/modules/
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

set(${target}_gdk_sources
    source/platform/gdk/gdkapplication.cpp
    source/platform/gdk/gdkapplication.h
    source/platform/gdk/gdkasync.cpp
    source/platform/gdk/gdkcommondirectories.cpp
    source/platform/gdk/gdkcommondirectories.h
    source/platform/gdk/gdkpreference.cpp
    source/platform/gdk/gdkpreference.h
    source/platform/gdk/gdkrunloop.cpp
    source/platform/gdk/gdkrunloop.h
    source/platform/gdk/gdkwindow.cpp
    source/platform/gdk/gdkwindow.h
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
if(LINUX)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SQLITE3 REQUIRED sqlite3)

    set(${target}_sources ${${target}_common_sources} ${${target}_gdk_sources})
endif()

##########################################################################################
add_library(${target} STATIC ${${target}_sources})

target_compile_definitions(${target} ${VSTGUI_COMPILE_DEFINITIONS})
vstgui_set_cxx_version(${target} 17)
vstgui_source_group_by_folder(${target})

if(VSTGUI_STANDALONE_EXAMPLES)
    add_subdirectory(examples)
endif()

if(LINUX)
    target_include_directories(${target} PRIVATE ${X11_INCLUDE_DIR})
    target_include_directories(${target} PRIVATE ${GTKMM3_INCLUDE_DIRS})
    target_include_directories(${target} PRIVATE ${FREETYPE_INCLUDE_DIRS})
    target_include_directories(${target} PRIVATE ${SQLITE3_INCLUDE_DIRS})
    target_link_libraries(${target} PRIVATE vstgui_uidescription ${LINUX_LIBRARIES} dl ${SQLITE3_LIBRARIES})
endif()

if(CMAKE_HOST_APPLE)
  target_compile_options(${target} PRIVATE -Wall -Werror)
endif()
