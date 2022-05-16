cmake_minimum_required(VERSION 3.12)

project(vstgui_ios_test)

if(NOT "${CMAKE_SYSTEM_NAME}" STREQUAL "iOS")
	message(FATAL_ERROR "Unsupported system (${CMAKE_SYSTEM_NAME}). Use -DCMAKE_SYSTEM_NAME=iOS when invoking cmake")
endif()

set(target "iostest")
set(sources
	"../../vstgui_ios.mm"
	"../../vstgui_uidescription.cpp"
	"iOS Standalone/AppDelegate.h"
	"iOS Standalone/AppDelegate.mm"
	"iOS Standalone/main.m"
	"iOS Standalone/ViewController.h"
	"iOS Standalone/ViewController.mm"
)

set(resources
	"iOS Standalone/ios_test.uidesc"
)

add_executable(${target} ${sources})
set_source_files_properties(${resources} PROPERTIES
  MACOSX_PACKAGE_LOCATION "Resources"
)
target_sources(${target} PRIVATE ${resources})
target_sources(${target} PRIVATE ${resources})
target_include_directories(${target} PRIVATE "../../../")
target_compile_features(${target} PUBLIC cxx_std_17)

target_link_libraries(${target}
	"-framework UIKit"
	"-framework QuartzCore"
	"-framework CoreGraphics"
	"-framework CoreText"
	"-framework ImageIO"
	"-framework MobileCoreServices"
	"-framework Accelerate"
)

set(bundleID net.sourceforge.vstgui.iostest)

set_target_properties(${target} PROPERTIES
	MACOSX_BUNDLE_BUNDLE_VERSION 1.0.0
	MACOSX_BUNDLE_SHORT_VERSION_STRING 1.0
	MACOSX_BUNDLE_GUI_IDENTIFIER ${bundleID}
	XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER ${bundleID}
)
