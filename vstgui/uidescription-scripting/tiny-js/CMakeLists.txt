project (tiny-js)
cmake_minimum_required (VERSION 2.6)

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

if (NOT CMAKE_BUILD_TYPE)
    set (CMAKE_BUILD_TYPE "Debug")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
else()
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
endif(NOT CMAKE_BUILD_TYPE)

if (NOT WIN32)
	include(CheckCXXCompilerFlag)
	CHECK_CXX_COMPILER_FLAG("-std=c++14" COMPILER_SUPPORTS_CXX14)
	if(COMPILER_SUPPORTS_CXX14)
    		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall")
	else()
    		message(FATAL_ERROR "Compiler ${CMAKE_CXX_COMPILER} has no C++14 support.")
	endif()
endif(NOT WIN32)

FILE(GLOB TINY_JS_HEADER_FILES
	${CMAKE_CURRENT_LIST_DIR}/TinyJS.h
)

FILE(GLOB TINY_JS_SOURCE_FILES
	${CMAKE_CURRENT_LIST_DIR}/TinyJS.cpp
    ${CMAKE_CURRENT_LIST_DIR}/TinyJS_Functions.cpp
    ${CMAKE_CURRENT_LIST_DIR}/TinyJS_MathFunctions.cpp
)

add_library(tiny-js STATIC ${TINY_JS_HEADER_FILES} ${TINY_JS_SOURCE_FILES})

ADD_EXECUTABLE(tiny-js-cli Script.cpp ${TINY_JS_SOURCE_FILES})

ADD_EXECUTABLE(tiny-js-tests run_tests.cpp ${TINY_JS_SOURCE_FILES})

add_custom_command(
	TARGET tiny-js-tests POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
			${CMAKE_SOURCE_DIR}/tests
			${CMAKE_CURRENT_BINARY_DIR}/tests)