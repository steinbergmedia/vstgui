set(TargetName uidesccompressor)

set(${TargetName}_sources
    main.cpp
)

set(${TargetName}_PLATFORM_LIBS "")

if(CMAKE_HOST_APPLE)
  set(${TargetName}_PLATFORM_LIBS
    "-framework Cocoa"
    "-framework OpenGL"
    "-framework QuartzCore"
    "-framework Accelerate"
    "-framework CoreAudio"
  )
endif()

add_executable(${TargetName}
  ${${TargetName}_sources}
)
target_link_libraries(${TargetName}
  vstgui
  vstgui_uidescription
  ${${TargetName}_PLATFORM_LIBS}
)
target_include_directories(${TargetName} PRIVATE ../../../)

vstgui_set_cxx_version(${TargetName} 14)
set_target_properties(${TargetName} PROPERTIES ${APP_PROPERTIES} ${VSTGUI_TOOLS_FOLDER})
target_compile_definitions(${TargetName} ${VSTGUI_COMPILE_DEFINITIONS})
