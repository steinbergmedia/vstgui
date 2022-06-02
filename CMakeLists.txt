##########################################################################################
cmake_minimum_required(VERSION 3.5)

if(NOT PROJECT_NAME)
    set(CMAKE_OSX_DEPLOYMENT_TARGET CACHE STRING 10.12)

    project(vstgui)
    set(VSTGUI_MAIN_PROJECT_BUILD 1)
    if(CMAKE_CONFIGURATION_TYPES)
        set(CMAKE_CONFIGURATION_TYPES Debug Release ReleaseLTO)
    endif()
endif()

add_subdirectory(vstgui)
