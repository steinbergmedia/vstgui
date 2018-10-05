###########################################################################################
get_filename_component(PkgInfoResource "${CMAKE_CURRENT_SOURCE_DIR}/cmake/resources/PkgInfo" ABSOLUTE)

###########################################################################################
if(LINUX)
  pkg_check_modules(GTKMM3 REQUIRED gtkmm-3.0)
endif(LINUX)

###########################################################################################
function(vstgui_add_executable target sources resources)

  if(MSVC)
    add_executable(${target} WIN32 ${sources})
    set_target_properties(${target} PROPERTIES LINK_FLAGS "/INCLUDE:wWinMain")
  endif(MSVC)

  if(LINUX)
    add_executable(${target} ${sources})
    get_target_property(OUTPUTDIR ${target} RUNTIME_OUTPUT_DIRECTORY)
    set_target_properties(${target} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${OUTPUTDIR}/${target}")
    set(PLATFORM_LIBRARIES ${GTKMM3_LIBRARIES})

    set(resource_folder "${OUTPUTDIR}/${target}/Resources")
        if(NOT EXISTS ${resource_folder})
          add_custom_command(TARGET ${target} PRE_LINK
              COMMAND ${CMAKE_COMMAND} -E make_directory
              "${resource_folder}"
          )
        endif()

    foreach(resource ${resources})
      message(${resource})
      add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_CURRENT_LIST_DIR}/${resource}"
        "${resource_folder}"
      )
    endforeach(resource ${resources})
    
  endif(LINUX)

  if(CMAKE_HOST_APPLE)
    set_source_files_properties(${resources} PROPERTIES
      MACOSX_PACKAGE_LOCATION "Resources"
    )
    set_source_files_properties(${PkgInfoResource} PROPERTIES
      MACOSX_PACKAGE_LOCATION "."
    )
    set(resources ${resources} ${PkgInfoResource})
    add_executable(${target} ${sources} ${resources})
    set(PLATFORM_LIBRARIES
      "-framework Cocoa"
      "-framework OpenGL"
      "-framework QuartzCore"
      "-framework Accelerate"
    )
    set_target_properties(${target} PROPERTIES
      MACOSX_BUNDLE TRUE
      OUTPUT_NAME "${target}"
    )
  endif(CMAKE_HOST_APPLE)

  target_link_libraries(${target}
    vstgui
    vstgui_uidescription
    vstgui_standalone
    ${PLATFORM_LIBRARIES}
  )
  target_compile_definitions(${target} ${VSTGUI_COMPILE_DEFINITIONS})
endfunction()

###########################################################################################
function(vstgui_set_target_infoplist target infoplist)
  if(CMAKE_HOST_APPLE)
    get_filename_component(InfoPlistFile "${infoplist}" ABSOLUTE)
    set_target_properties(${target} PROPERTIES
      MACOSX_BUNDLE_INFO_PLIST ${InfoPlistFile}
    )
  endif(CMAKE_HOST_APPLE)
endfunction()

###########################################################################################
function(vstgui_set_target_rcfile target rcfile)
  if(MSVC)
    target_sources(${target} PRIVATE ${rcfile})
  endif(MSVC)
endfunction()
