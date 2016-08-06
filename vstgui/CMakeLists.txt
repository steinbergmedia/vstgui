##########################################################################################
cmake_minimum_required(VERSION 3.4.1)
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
add_subdirectory(lib)
add_subdirectory(uidescription)
if(NOT VSTGUI_DISABLE_STANDALONE)
  add_subdirectory(standalone)
endif()
