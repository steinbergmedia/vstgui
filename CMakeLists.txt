##########################################################################################
cmake_minimum_required(VERSION 3.5)

if(NOT PROJECT_NAME)
    project(vstgui)
    set(VSTGUI_MAIN_PROJECT_BUILD 1)
    set(CMAKE_OSX_DEPLOYMENT_TARGET 10.9)
endif()

add_subdirectory(vstgui)
