##########################################################################################
cmake_minimum_required(VERSION 3.5)
project(vstgui)

##########################################################################################
macro(SOURCE_GROUP_BY_FOLDER target)
  if(CMAKE_CONFIGURATION_TYPES)
    set(SOURCE_GROUP_DELIMITER "/")
    set(last_dir "")
    set(files "")
    foreach(file ${${target}_sources})
      get_filename_component(dir "${file}" DIRECTORY)
      string(FIND "${dir}" ${target} isTargetFolder)
      if (${isTargetFolder} EQUAL 0)
        string(LENGTH ${target} offset)
        string(SUBSTRING "${dir}" ${offset} -1 dir)
      endif(${isTargetFolder} EQUAL 0)
      if (NOT "${dir}" STREQUAL "${last_dir}")
        if (files)
          source_group("${last_dir}" FILES ${files})
        endif (files)
        set(files "")
      endif (NOT "${dir}" STREQUAL "${last_dir}")
      set(files ${files} ${file})
      set(last_dir "${dir}")
    endforeach(file)
    if (files)
      source_group("${last_dir}" FILES ${files})
    endif (files)
  endif(CMAKE_CONFIGURATION_TYPES)
endmacro(SOURCE_GROUP_BY_FOLDER)

##########################################################################################
if(CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_CONFIGURATION_TYPES Debug Release)
  set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif()

##########################################################################################
set(VSTGUI_COMPILE_DEFINITIONS_DEBUG "VSTGUI_LIVE_EDITING;DEBUG")
set(VSTGUI_COMPILE_DEFINITIONS_RELEASE "NDEBUG;RELEASE")
if(MSVC)
  set(VSTGUI_COMPILE_DEFINITIONS_DEBUG "${VSTGUI_COMPILE_DEFINITIONS_DEBUG};_CRT_SECURE_NO_WARNINGS")
  set(VSTGUI_COMPILE_DEFINITIONS_RELEASE "${VSTGUI_COMPILE_DEFINITIONS_RELEASE};_CRT_SECURE_NO_WARNINGS")
endif()

set(VSTGUI_COMPILE_DEFINITIONS PRIVATE
    $<$<CONFIG:Debug>:${VSTGUI_COMPILE_DEFINITIONS_DEBUG}>
    $<$<CONFIG:Release>:${VSTGUI_COMPILE_DEFINITIONS_RELEASE}>
)

##########################################################################################
if(NOT CMAKE_CONFIGURATION_TYPES)
  if(NOT CMAKE_BUILD_TYPE)
  	set(CMAKE_BUILD_TYPE Debug)
  endif()

  message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY
  $<$<CONFIG:Debug>:${CMAKE_BINARY_DIR}/Debug/>$<$<CONFIG:Release>:${CMAKE_BINARY_DIR}/Release/>
)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY
  $<$<CONFIG:Debug>:${CMAKE_BINARY_DIR}/Debug/libs/>$<$<CONFIG:Release>:${CMAKE_BINARY_DIR}/Release/libs/>
)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY
  $<$<CONFIG:Debug>:${CMAKE_BINARY_DIR}/Debug/libs>$<$<CONFIG:Release>:${CMAKE_BINARY_DIR}/Release/libs>
)

##########################################################################################
add_subdirectory(lib)
add_subdirectory(uidescription)
if(NOT VSTGUI_DISABLE_STANDALONE)
  add_subdirectory(standalone)
endif()

##########################################################################################
# VSTGUI Simple Standalone Application
##########################################################################################
if(NOT VSTGUI_DISABLE_SIMPLE_STANDALONE_APP)
  set(target simple_standalone)

  set(${target}_sources
    "tests/simple_standalone/source/myapp.cpp"
  )

  ##########################################################################################
  if(CMAKE_HOST_APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET 10.9)
  	set(BUNDLE_PKG_INFO
      "tests/simple_standalone/resource/PkgInfo"
  	)
  	set(simple_standalone_resources
  		"tests/simple_standalone/resource/Window.uidesc"
  	)
  	set_source_files_properties(${BUNDLE_PKG_INFO} PROPERTIES
  		MACOSX_PACKAGE_LOCATION "."
  	)
    set_source_files_properties(${simple_standalone_resources} PROPERTIES
      MACOSX_PACKAGE_LOCATION "Resources"
    )
    set(simple_standalone_resources ${simple_standalone_resources} ${BUNDLE_PKG_INFO})
    
  	set(${target}_PLATFORM_LIBS
  		"-framework Cocoa"
  		"-framework OpenGL"
  		"-framework QuartzCore"
  		"-framework Accelerate"
  	)
  	get_filename_component(InfoPlistFile "tests/simple_standalone/resource/Info.plist" ABSOLUTE)
  	set(APP_PROPERTIES
  		MACOSX_BUNDLE TRUE
  		MACOSX_BUNDLE_INFO_PLIST ${InfoPlistFile}
  		OUTPUT_NAME "simple_standalone"
  	)
  endif()

  ##########################################################################################
  if(MSVC)
    set(simple_standalone_resources
      "tests/simple_standalone/resource/simple_standalone.rc"
    )
    set(APP_PROPERTIES
      LINK_FLAGS "/SUBSYSTEM:windows /INCLUDE:wWinMain"
    )
  endif()

  ##########################################################################################
  include_directories(../)
  add_executable(${target}
    ${${target}_sources}
    ${simple_standalone_resources}
  )
  target_link_libraries(${target}
  	vstgui
    vstgui_uidescription
    vstgui_standalone
  	${${target}_PLATFORM_LIBS}
  )

  set_target_properties(${target} PROPERTIES ${APP_PROPERTIES})
  set_property(TARGET ${target} PROPERTY CXX_STANDARD 14)
  set_property(TARGET ${target} PROPERTY CXX_STANDARD_REQUIRED ON)
  set_property(TARGET ${target} PROPERTY CMAKE_CXX_STANDARD_REQUIRED ON)

  target_compile_definitions(${target} ${VSTGUI_COMPILE_DEFINITIONS})
  
  SOURCE_GROUP_BY_FOLDER(${target})
endif()
