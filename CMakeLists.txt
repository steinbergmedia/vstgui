##########################################################################################
cmake_minimum_required(VERSION 3.5)

if(NOT PROJECT_NAME)
    project(vstgui)
    set(VSTGUI_MAIN_PROJECT_BUILD 1)
    if(NOT CMAKE_OSX_DEPLOYMENT_TARGET)
	    set(CMAKE_OSX_DEPLOYMENT_TARGET 10.9)
	endif()
    if(CMAKE_CONFIGURATION_TYPES)
        set(CMAKE_CONFIGURATION_TYPES Debug Release ReleaseLTO)
    endif()
endif()

add_subdirectory(vstgui)
