##########################################################################################
# VSTGUI base64codecspeed
##########################################################################################
set(target base64codecspeed)

set(${target}_sources
  "main.cpp"
)

##########################################################################################
include_directories(../../../)
add_executable(${target}
  ${${target}_sources}
)
target_link_libraries(${target}
	${${target}_PLATFORM_LIBS}
)

vstgui_set_cxx_version(${target} 14)
set_target_properties(${target} PROPERTIES ${APP_PROPERTIES} FOLDER Tests)
target_compile_definitions(${target} ${VSTGUI_COMPILE_DEFINITIONS})
