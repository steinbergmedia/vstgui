##########################################################################################
# VSTGUI Unittests
##########################################################################################

set(target unittests)

set(VSTGUI_TEST_BASE "")
#set(VSTGUI_TEST_BASE "${VSTGUI_TEST_BASE}")

set(${target}_sources
	"${VSTGUI_TEST_BASE}unittests.cpp"
	"${VSTGUI_TEST_BASE}unittests.h"
	"${VSTGUI_TEST_BASE}lib/animation/animations_test.cpp"
	"${VSTGUI_TEST_BASE}lib/animation/animator_test.cpp"
	"${VSTGUI_TEST_BASE}lib/animation/timingfunction_tests.cpp"
	"${VSTGUI_TEST_BASE}lib/controls/ccheckbox_test.cpp"
	"${VSTGUI_TEST_BASE}lib/controls/ccontrol_test.cpp"
	"${VSTGUI_TEST_BASE}lib/controls/conoffbutton_test.cpp"
	"${VSTGUI_TEST_BASE}lib/controls/csegmentbutton_test.cpp"
	"${VSTGUI_TEST_BASE}lib/controls/ctextbutton_test.cpp"
	"${VSTGUI_TEST_BASE}lib/controls/cxypad_test.cpp"
	"${VSTGUI_TEST_BASE}lib/cbitmap_test.cpp"
	"${VSTGUI_TEST_BASE}lib/cbuttonstate_test.cpp"
	"${VSTGUI_TEST_BASE}lib/ccolor_test.cpp"
	"${VSTGUI_TEST_BASE}lib/cframe_test.cpp"
	"${VSTGUI_TEST_BASE}lib/clinestyle_test.cpp"
	"${VSTGUI_TEST_BASE}lib/cpoint_test.cpp"
	"${VSTGUI_TEST_BASE}lib/crect_test.cpp"
	"${VSTGUI_TEST_BASE}lib/csplitview_test.cpp"
	"${VSTGUI_TEST_BASE}lib/cview_test.cpp"
	"${VSTGUI_TEST_BASE}lib/cviewcontainer_test.cpp"
	"${VSTGUI_TEST_BASE}lib/idependency_test.cpp"
	"${VSTGUI_TEST_BASE}lib/platform_helper.h"
	"${VSTGUI_TEST_BASE}lib/utf8string_test.cpp"
	"${VSTGUI_TEST_BASE}lib/utf8stringview_test.cpp"
	"${VSTGUI_TEST_BASE}uidescription/uiviewcreator/canimationsplashscreencreator_test.cpp"
	"${VSTGUI_TEST_BASE}uidescription/uiviewcreator/canimknobcreator_test.cpp"
	"${VSTGUI_TEST_BASE}uidescription/uiviewcreator/ccheckboxcreator_test.cpp"
	"${VSTGUI_TEST_BASE}uidescription/uiviewcreator/ccontrolcreator_test.cpp"
	"${VSTGUI_TEST_BASE}uidescription/uiviewcreator/cgradientviewcreator_test.cpp"
	"${VSTGUI_TEST_BASE}uidescription/uiviewcreator/chorizontalswitchcreator_test.cpp"
	"${VSTGUI_TEST_BASE}uidescription/uiviewcreator/ckickbuttoncreator_test.cpp"
	"${VSTGUI_TEST_BASE}uidescription/uiviewcreator/cknobcreator_test.cpp"
	"${VSTGUI_TEST_BASE}uidescription/uiviewcreator/clayeredviewcontainercreator_test.cpp"
	"${VSTGUI_TEST_BASE}uidescription/uiviewcreator/cmoviebitmapcreator_test.cpp"
	"${VSTGUI_TEST_BASE}uidescription/uiviewcreator/cmoviebuttoncreator_test.cpp"
	"${VSTGUI_TEST_BASE}uidescription/uiviewcreator/conoffbuttoncreator_test.cpp"
	"${VSTGUI_TEST_BASE}uidescription/uiviewcreator/coptionmenucreator_test.cpp"
	"${VSTGUI_TEST_BASE}uidescription/uiviewcreator/cparamdisplaycreator_test.cpp"
	"${VSTGUI_TEST_BASE}uidescription/uiviewcreator/crockerswitchcreator_test.cpp"
	"${VSTGUI_TEST_BASE}uidescription/uiviewcreator/crowcolumnviewcreator_test.cpp"
	"${VSTGUI_TEST_BASE}uidescription/uiviewcreator/cscrollviewcreator_test.cpp"
	"${VSTGUI_TEST_BASE}uidescription/uiviewcreator/csegmentbuttoncreator_test.cpp"
	"${VSTGUI_TEST_BASE}uidescription/uiviewcreator/cshadowviewcontainercreator_test.cpp"
	"${VSTGUI_TEST_BASE}uidescription/uiviewcreator/cslidercreator_test.cpp"
	"${VSTGUI_TEST_BASE}uidescription/uiviewcreator/csplitviewcreator_test.cpp"
	"${VSTGUI_TEST_BASE}uidescription/uiviewcreator/ctextbuttoncreator_test.cpp"
	"${VSTGUI_TEST_BASE}uidescription/uiviewcreator/ctexteditcreator_test.cpp"
	"${VSTGUI_TEST_BASE}uidescription/uiviewcreator/ctextlabelcreator_test.cpp"
	"${VSTGUI_TEST_BASE}uidescription/uiviewcreator/cverticalswitchcreator_test.cpp"
	"${VSTGUI_TEST_BASE}uidescription/uiviewcreator/cviewcontainercreator_test.cpp"
	"${VSTGUI_TEST_BASE}uidescription/uiviewcreator/cviewcreator_test.cpp"
	"${VSTGUI_TEST_BASE}uidescription/uiviewcreator/cvumetercreator_test.cpp"
	"${VSTGUI_TEST_BASE}uidescription/uiviewcreator/cxypadcreator_test.cpp"
	"${VSTGUI_TEST_BASE}uidescription/uiviewcreator/helpers.h"
	"${VSTGUI_TEST_BASE}uidescription/uiviewcreator/uiviewswitchcontainercreator_test.cpp"
	"${VSTGUI_TEST_BASE}uidescription/base64codec.cpp"
	"${VSTGUI_TEST_BASE}uidescription/cstream_test.cpp"
	"${VSTGUI_TEST_BASE}uidescription/delegationcontroller_test.cpp"
	"${VSTGUI_TEST_BASE}uidescription/uiattributes_test.cpp"
	"${VSTGUI_TEST_BASE}uidescription/uidescription_test.cpp"
	"${VSTGUI_TEST_BASE}uidescription/uidescriptionadapter.h"
	"${VSTGUI_TEST_BASE}uidescription/uiviewfactory_test.cpp"
	"${VSTGUI_TEST_BASE}uidescription/uiviewswitchcontainer_test.cpp"
	"${VSTGUI_TEST_BASE}uidescription/xmlparser_test.cpp"
	"${VSTGUI_TEST_BASE}../../vstgui_uidescription.cpp"
)

##########################################################################################
if(CMAKE_HOST_APPLE)
	set(${target}_sources
		${${target}_sources}
		"${VSTGUI_TEST_BASE}lib/platform_helper_mac.mm"
		"${VSTGUI_TEST_BASE}../../vstgui_mac.mm"
	)

	set(${target}_PLATFORM_LIBS
		"-framework Cocoa"
		"-framework OpenGL"
		"-framework QuartzCore"
		"-framework Accelerate"
	)
endif()

##########################################################################################
if(MSVC)
	set(${target}_sources
		${${target}_sources}
		"${VSTGUI_TEST_BASE}lib/platform_helper_win32.cpp"
		"${VSTGUI_TEST_BASE}../../vstgui_win32.cpp"
	)
endif()

##########################################################################################
if(UNIX AND NOT CMAKE_HOST_APPLE)
	set(${target}_sources
		${${target}_sources}
		"${VSTGUI_TEST_BASE}lib/platform_helper_linux.cpp"
		"${VSTGUI_TEST_BASE}../../vstgui_linux.cpp"
	)
	set(${target}_PLATFORM_LIBS
		${LINUX_LIBRARIES}
		stdc++fs
		pthread
		dl
	)
endif()

##########################################################################################
#include_directories(../)
add_executable(${target} ${${target}_sources})
target_link_libraries(${target}
	${${target}_PLATFORM_LIBS}
)

vstgui_set_cxx_version(${target} 11)
target_compile_definitions(${target} ${VSTGUI_COMPILE_DEFINITIONS} ENABLE_UNIT_TESTS=1 VSTGUI_LIVE_EDITING=1)
vstgui_source_group_by_folder(${target})

add_custom_command(TARGET ${target} POST_BUILD COMMAND "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/unittests")

##########################################################################################
if(UNIX AND NOT CMAKE_HOST_APPLE)
	target_include_directories(${target} PRIVATE ${X11_INCLUDE_DIR})
	target_include_directories(${target} PRIVATE ${FREETYPE_INCLUDE_DIRS})
endif()
