###########################################################################################
get_filename_component(PkgInfoResource "${CMAKE_CURRENT_SOURCE_DIR}/cmake/resources/PkgInfo" ABSOLUTE)

###########################################################################################
if(LINUX)
  pkg_check_modules(GTKMM3 REQUIRED gtkmm-3.0)
endif(LINUX)

###########################################################################################
function(vstgui_add_executable target sources)

  if(MSVC)
    add_executable(${target} WIN32 ${sources})
    set_target_properties(${target} PROPERTIES LINK_FLAGS "/INCLUDE:wWinMain")
    get_target_property(OUTPUTDIR ${target} RUNTIME_OUTPUT_DIRECTORY)
    set_target_properties(${target} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${OUTPUTDIR}/${target}")
  endif(MSVC)

  if(LINUX)
    add_executable(${target} ${sources})
    get_target_property(OUTPUTDIR ${target} RUNTIME_OUTPUT_DIRECTORY)
    set_target_properties(${target} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${OUTPUTDIR}/${target}")
    set(PLATFORM_LIBRARIES ${GTKMM3_LIBRARIES})
  endif(LINUX)

  if(CMAKE_HOST_APPLE)
    set_source_files_properties(${PkgInfoResource} PROPERTIES
      MACOSX_PACKAGE_LOCATION "."
    )
    add_executable(${target} ${sources} ${PkgInfoResource})
    set(PLATFORM_LIBRARIES
      "-framework Cocoa"
      "-framework OpenGL"
      "-framework QuartzCore"
      "-framework Accelerate"
    )
    set_target_properties(${target} PROPERTIES
      MACOSX_BUNDLE TRUE
      XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT $<$<CONFIG:Debug>:dwarf>$<$<NOT:$<CONFIG:Debug>>:dwarf-with-dsym>
      XCODE_ATTRIBUTE_DEPLOYMENT_POSTPROCESSING $<$<CONFIG:Debug>:NO>$<$<NOT:$<CONFIG:Debug>>:YES>
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

  if(ARGC GREATER 2)
    vstgui_add_resources(${target} "${ARGV2}")
    message(DEPRECATION "Please use vstgui_add_resources to add resources to an executable now.")
  endif()

endfunction()

###########################################################################################
function(vstgui_add_resources target resources)
  set(destination "Resources")
  if(ARGC GREATER 2)
    set(destination "${destination}/${ARGV2}")
  endif()
  if(CMAKE_HOST_APPLE)
    set_source_files_properties(${resources} PROPERTIES
      MACOSX_PACKAGE_LOCATION "${destination}"
    )
    target_sources(${target} PRIVATE ${resources})
  else()
    get_target_property(OUTPUTDIR ${target} RUNTIME_OUTPUT_DIRECTORY)
    set(destination "${OUTPUTDIR}/${destination}")
    if(NOT EXISTS ${destination})
      add_custom_command(TARGET ${target} PRE_BUILD
          COMMAND ${CMAKE_COMMAND} -E make_directory
          "${destination}"
      )
    endif()
    foreach(resource ${resources})
      get_filename_component(sourcePath "${resource}" ABSOLUTE "${CMAKE_CURRENT_LIST_DIR}")
      if(IS_DIRECTORY "${sourcePath}")
		get_filename_component(directoryName "${resource}" NAME)
        add_custom_command(TARGET ${target} POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E copy_directory
          "${sourcePath}"
          "${destination}/${directoryName}"
        )
	  else()
        add_custom_command(TARGET ${target} POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E copy
          "${sourcePath}"
          "${destination}"
        )
	  endif()
    endforeach(resource ${resources})
  endif()  
endfunction()

###########################################################################################
function(vstgui_set_target_infoplist target infoplist)
  if(CMAKE_HOST_APPLE)
    get_filename_component(InfoPlistFile "${infoplist}" ABSOLUTE)
    get_filename_component(IncludeDir "${InfoPlistFile}" DIRECTORY)
    set_target_properties(${target} PROPERTIES
      MACOSX_BUNDLE_INFO_PLIST ${InfoPlistFile}
      XCODE_ATTRIBUTE_INFOPLIST_PREPROCESS YES
      XCODE_ATTRIBUTE_INFOPLIST_OTHER_PREPROCESSOR_FLAGS "-I${IncludeDir}"
      XCODE_ATTRIBUTE_INFOPLIST_PREPROCESSOR_DEFINITIONS __plist_preprocessor__
    )
  endif(CMAKE_HOST_APPLE)
endfunction()

###########################################################################################
function(vstgui_set_target_rcfile target rcfile)
  if(MSVC)
    target_sources(${target} PRIVATE ${rcfile})
  endif(MSVC)
endfunction()
