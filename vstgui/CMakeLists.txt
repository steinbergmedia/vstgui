##########################################################################################
cmake_minimum_required(VERSION 3.5)

if(NOT PROJECT_NAME)
    set(CMAKE_OSX_DEPLOYMENT_TARGET 10.14 CACHE STRING "")
    project(vstgui)
    set(VSTGUI_MAIN_PROJECT_BUILD 1)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/modules/")

include(vstgui_init)

##########################################################################################
function(vstgui_set_cxx_version target version)
    target_compile_features(${target} PUBLIC cxx_std_${version})
    set_property(TARGET ${target} PROPERTY CXX_STANDARD ${version})
    set_property(TARGET ${target} PROPERTY CXX_STANDARD_REQUIRED ON)
    set_property(TARGET ${target} PROPERTY CMAKE_CXX_STANDARD_REQUIRED ON)
    if(APPLE)
        target_compile_options(${target} PUBLIC "-stdlib=libc++")
    endif()
endfunction(vstgui_set_cxx_version target version)

##########################################################################################
function(vstgui_source_group_by_folder target)
    if(CMAKE_CONFIGURATION_TYPES)
        set(SOURCE_GROUP_DELIMITER "/")
        set(last_dir "")
        set(files "")
        foreach(file ${${target}_sources})
            get_filename_component(dir "${file}" DIRECTORY)
            string(FIND "${dir}" ${target} isTargetFolder)
            if(${isTargetFolder} EQUAL 0)
                string(LENGTH ${target} offset)
                string(SUBSTRING "${dir}" ${offset} -1 dir)
            endif(${isTargetFolder} EQUAL 0)
            if(NOT "${dir}" STREQUAL "${last_dir}")
                if(files)
                    source_group("${last_dir}" FILES ${files})
                endif(files)
                set(files "")
            endif(NOT "${dir}" STREQUAL "${last_dir}")
            set(files ${files} ${file})
            set(last_dir "${dir}")
        endforeach(file)
        if(files)
            source_group("${last_dir}" FILES ${files})
        endif(files)
    endif(CMAKE_CONFIGURATION_TYPES)
endfunction(vstgui_source_group_by_folder)

##########################################################################################
if(LINUX)
    find_package(X11 REQUIRED)
    find_package(Freetype REQUIRED)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LIBXCB REQUIRED xcb)
    pkg_check_modules(LIBXCB_UTIL REQUIRED xcb-util)
    pkg_check_modules(LIBXCB_CURSOR REQUIRED xcb-cursor)
    pkg_check_modules(LIBXCB_KEYSYMS REQUIRED xcb-keysyms)
    pkg_check_modules(LIBXCB_XKB REQUIRED xcb-xkb)
    pkg_check_modules(LIBXKB_COMMON REQUIRED xkbcommon)
    pkg_check_modules(LIBXKB_COMMON_X11 REQUIRED xkbcommon-x11)
    pkg_check_modules(GLIB REQUIRED glib-2.0)
    pkg_check_modules(CAIRO REQUIRED cairo)
    pkg_check_modules(PANGO REQUIRED pangocairo pangoft2)
    pkg_check_modules(FONTCONFIG REQUIRED fontconfig)
    set(LINUX_LIBRARIES
        ${X11_LIBRARIES}
        ${FREETYPE_LIBRARIES}
        ${LIBXCB_LIBRARIES}
        ${LIBXCB_UTIL_LIBRARIES}
        ${LIBXCB_CURSOR_LIBRARIES}
        ${LIBXCB_KEYSYMS_LIBRARIES}
        ${LIBXCB_XKB_LIBRARIES}
        ${LIBXKB_COMMON_LIBRARIES}
        ${LIBXKB_COMMON_X11_LIBRARIES}
        ${GLIB_LIBRARIES}
        ${CAIRO_LIBRARIES}
        ${PANGO_LIBRARIES}
        ${FONTCONFIG_LIBRARIES}
        dl
    )
endif()

##########################################################################################
if(NOT CMAKE_CONFIGURATION_TYPES)
    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE Debug)
    endif()

    message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
endif()

if(VSTGUI_MAIN_PROJECT_BUILD)
    message(STATUS "Building only vstgui")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY
        $<$<CONFIG:Debug>:${CMAKE_BINARY_DIR}/Debug/>$<$<CONFIG:Release>:${CMAKE_BINARY_DIR}/Release/>$<$<CONFIG:ReleaseLTO>:${CMAKE_BINARY_DIR}/ReleaseLTO/>
    )
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY
        $<$<CONFIG:Debug>:${CMAKE_BINARY_DIR}/Debug/libs/>$<$<CONFIG:Release>:${CMAKE_BINARY_DIR}/Release/libs/>$<$<CONFIG:ReleaseLTO>:${CMAKE_BINARY_DIR}/ReleaseLTO/libs/>
    )
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY
        $<$<CONFIG:Debug>:${CMAKE_BINARY_DIR}/Debug/libs>$<$<CONFIG:Release>:${CMAKE_BINARY_DIR}/Release/libs>$<$<CONFIG:ReleaseLTO>:${CMAKE_BINARY_DIR}/ReleaseLTO/libs>
    )
endif()

##########################################################################################
add_subdirectory(lib)
add_subdirectory(uidescription)

if(LINUX)
    set(VSTGUI_DISABLE_UNITTESTS  1)
endif()

if(NOT DEFINED VSTGUI_STANDALONE)
    option(VSTGUI_STANDALONE "VSTGUI Standalone library" ON)
    if(NOT DEFINED VSTGUI_STANDALONE_EXAMPLES)
        option(VSTGUI_STANDALONE_EXAMPLES "VSTGUI Standalone examples" ON)
    endif()
endif()

if(NOT VSTGUI_STANDALONE AND VSTGUI_STANDALONE_EXAMPLES)
    set(VSTGUI_STANDALONE_EXAMPLES OFF)
endif()

if(NOT DEFINED VSTGUI_TOOLS)
    option(VSTGUI_TOOLS "Build VSTGUI Tools" ON)
endif()

if(VSTGUI_STANDALONE)
    add_subdirectory(standalone)
    if(NOT VSTGUI_DISABLE_UNITTESTS)
        add_subdirectory(tests/gfxtest)
        add_subdirectory(tests/base64codecspeed)
    endif()
endif()
if(NOT VSTGUI_DISABLE_UNITTESTS)
    add_subdirectory(tests)
endif()
if(VSTGUI_TOOLS)
    add_subdirectory(tools)
endif()

get_directory_property(hasParent PARENT_DIRECTORY)
if(hasParent)
    set(VSTGUI_COMPILE_DEFINITIONS ${VSTGUI_COMPILE_DEFINITIONS} PARENT_SCOPE)
    set(VSTGUI_LTO_COMPILER_FLAGS ${VSTGUI_LTO_COMPILER_FLAGS} PARENT_SCOPE)
endif()
